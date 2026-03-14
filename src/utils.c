#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "include/archium.h"

typedef struct {
  int *running;
  const char *message;
} spinner_data_t;

void handle_signal(int signal) {
  if (signal == SIGINT || signal == SIGTERM || signal == SIGABRT) {
    printf("\nSignal %d received. Exiting gracefully.\n", signal);
    cleanup_cached_commands();
    archium_plugin_cleanup();
    exit(EXIT_SUCCESS);
  }
}

static int get_terminal_width(void) {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
    return w.ws_col;
  }
  return 80;
}

static int command_uses_pacman_like_output(const char *command) {
  if (!command) {
    return 0;
  }

  return strstr(command, "pacman") != NULL || strstr(command, "yay") != NULL ||
         strstr(command, "paru") != NULL;
}

static int command_likely_requires_sudo(const char *command) {
  if (!command) {
    return 0;
  }

  return strstr(command, " -S") != NULL || strstr(command, " -R") != NULL ||
         strstr(command, " -U") != NULL;
}

static void ensure_sudo_credentials_for_custom_output(const char *command) {
  if (config.use_native_output || config.batch_mode || config.json_output) {
    return;
  }

  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
    return;
  }

  if (!command_uses_pacman_like_output(command) ||
      !command_likely_requires_sudo(command)) {
    return;
  }

  if (system("sudo -n -v >/dev/null 2>&1") == 0) {
    return;
  }

  fflush(stdout);
  (void)system("sudo -v");
}

static int parse_fraction_progress(const char *line, int *current, int *total) {
  const char *cursor = line;
  while ((cursor = strchr(cursor, '(')) != NULL) {
    int parsed_current = 0;
    int parsed_total = 0;
    if (sscanf(cursor, "(%d/%d)", &parsed_current, &parsed_total) == 2 &&
        parsed_total > 0 && parsed_current >= 0) {
      *current = parsed_current;
      *total = parsed_total;
      return 1;
    }
    cursor++;
  }

  return 0;
}

static int parse_percentage_progress(const char *line, int *percentage) {
  size_t line_len = strlen(line);
  for (size_t i = 0; i < line_len; i++) {
    if (line[i] != '%') {
      continue;
    }

    size_t start = i;
    while (start > 0 && isdigit((unsigned char)line[start - 1])) {
      start--;
    }

    if (start == i) {
      continue;
    }

    char number[8];
    size_t number_len = i - start;
    if (number_len >= sizeof(number)) {
      continue;
    }

    memcpy(number, line + start, number_len);
    number[number_len] = '\0';

    int parsed_percentage = atoi(number);
    if (parsed_percentage >= 0 && parsed_percentage <= 100) {
      *percentage = parsed_percentage;
      return 1;
    }
  }

  return 0;
}

static void update_progress_from_line(const char *line, int prefer_fraction,
                                      int *current, int *total,
                                      int *has_progress) {
  int parsed_current = 0;
  int parsed_total = 0;
  int parsed_percentage = 0;

  if (prefer_fraction &&
      parse_fraction_progress(line, &parsed_current, &parsed_total)) {
    *current = parsed_current;
    *total = parsed_total;
    *has_progress = 1;
    return;
  }

  if (parse_percentage_progress(line, &parsed_percentage)) {
    *current = parsed_percentage;
    *total = 100;
    *has_progress = 1;
    return;
  }

  if (!prefer_fraction &&
      parse_fraction_progress(line, &parsed_current, &parsed_total)) {
    *current = parsed_current;
    *total = parsed_total;
    *has_progress = 1;
  }
}

static void render_enhanced_indicator(const char *message, int spinner_position,
                                      int has_progress, int current,
                                      int total) {
  if (has_progress && total > 0) {
    int bounded_current = current;
    if (bounded_current < 0) {
      bounded_current = 0;
    }
    if (bounded_current > total) {
      bounded_current = total;
    }
    show_progress_bar(bounded_current, total, message);
    return;
  }

  show_spinner(spinner_position, message);
}

void show_progress_bar(int current, int total, const char *prefix) {
  if (total <= 0) return;

  int terminal_width = get_terminal_width();
  int prefix_len = prefix ? strlen(prefix) : 0;
  int available_width = terminal_width - prefix_len - 20;
  if (available_width < 10) available_width = 10;

  float percentage = (float)current / total;
  int filled_width = (int)(percentage * available_width);

  printf("\r");
  if (prefix) {
    printf("%s ", prefix);
  }

  printf("[");
  for (int i = 0; i < available_width; i++) {
    if (i < filled_width) {
      printf("█");
    } else {
      printf("░");
    }
  }
  printf("] %3.0f%% (%d/%d)", percentage * 100, current, total);

  if (current == total) {
    printf("\n");
  }
  fflush(stdout);
}

static void *spinner_thread(void *arg) {
  spinner_data_t *data = (spinner_data_t *)arg;
  const char *spinner_chars = "|/-\\";
  int position = 0;

  while (*data->running) {
    printf("\r\033[34m%s\033[0m %c", data->message,
           spinner_chars[position % 4]);
    fflush(stdout);
    position++;
    usleep(100000);
  }

  printf("\r\033[K");
  fflush(stdout);
  return NULL;
}

void show_spinner(int position, const char *message) {
  const char *spinner_chars = "|/-\\";
  printf("\r%s %c", message ? message : "Processing",
         spinner_chars[position % 4]);
  fflush(stdout);
}

int execute_command_with_spinner(const char *command, const char *message) {
  int running = 1;
  pthread_t spinner_tid;
  spinner_data_t spinner_data = {&running, message ? message : "Processing"};

  if (config.batch_mode || config.json_output) {
    char silent_command[COMMAND_BUFFER_SIZE];
    snprintf(silent_command, sizeof(silent_command),
             "%s 2>/dev/null >/dev/null", command);
    return system(silent_command);
  }

  if (pthread_create(&spinner_tid, NULL, spinner_thread, &spinner_data) != 0) {
    return system(command);
  }

  char silent_command[COMMAND_BUFFER_SIZE];
  snprintf(silent_command, sizeof(silent_command), "%s 2>/dev/null >/dev/null",
           command);
  int result = system(silent_command);

  running = 0;
  pthread_join(spinner_tid, NULL);

  return result;
}

int execute_command_with_output_capture(const char *command,
                                        const char *message,
                                        char *output_buffer,
                                        size_t buffer_size) {
  const char *display_message = message ? message : "Processing";
  int use_interactive_indicator = isatty(STDOUT_FILENO);
  int prefer_fraction_progress = command_uses_pacman_like_output(command);
  int spinner_position = 0;
  int has_progress = 0;
  int current_progress = 0;
  int total_progress = 100;

  if (config.batch_mode || config.json_output) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) return -1;
    if (output_buffer && buffer_size > 0) {
      size_t bytes_read = fread(output_buffer, 1, buffer_size - 1, fp);
      output_buffer[bytes_read] = '\0';
    }
    return pclose(fp);
  }

  if (output_buffer && buffer_size > 0) {
    output_buffer[0] = '\0';
  }

  ensure_sudo_credentials_for_custom_output(command);

  char merged_command[COMMAND_BUFFER_SIZE + 32];
  if (snprintf(merged_command, sizeof(merged_command), "%s 2>&1", command) >=
      (int)sizeof(merged_command)) {
    return system(command);
  }

  FILE *fp = popen(merged_command, "r");
  if (fp == NULL) {
    return -1;
  }

  int fd = fileno(fp);
  int original_flags = fcntl(fd, F_GETFL, 0);
  if (original_flags != -1) {
    (void)fcntl(fd, F_SETFL, original_flags | O_NONBLOCK);
  }

  char line_buffer[1024];
  size_t line_length = 0;
  size_t captured = 0;

  while (1) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int ready = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ready < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }

    if (ready == 0) {
      if (use_interactive_indicator) {
        render_enhanced_indicator(display_message, spinner_position,
                                  has_progress, current_progress,
                                  total_progress);
        spinner_position++;
      }
      continue;
    }

    char chunk[256];
    ssize_t bytes_read = read(fd, chunk, sizeof(chunk));
    if (bytes_read < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        continue;
      }
      break;
    }

    if (bytes_read == 0) {
      break;
    }

    for (ssize_t i = 0; i < bytes_read; i++) {
      unsigned char c = (unsigned char)chunk[i];

      if (output_buffer && buffer_size > 0 && captured < buffer_size - 1) {
        output_buffer[captured++] = (char)c;
        output_buffer[captured] = '\0';
      }

      if (c == '\n' || c == '\r') {
        line_buffer[line_length] = '\0';
        if (line_length > 0) {
          update_progress_from_line(line_buffer, prefer_fraction_progress,
                                    &current_progress, &total_progress,
                                    &has_progress);
        }
        line_length = 0;
        continue;
      }

      if ((isprint(c) || c == '\t') && line_length < sizeof(line_buffer) - 1) {
        line_buffer[line_length++] = (char)c;
      }
    }
  }

  if (line_length > 0) {
    line_buffer[line_length] = '\0';
    update_progress_from_line(line_buffer, prefer_fraction_progress,
                              &current_progress, &total_progress,
                              &has_progress);
  }

  if (use_interactive_indicator) {
    if (has_progress && total_progress > 0) {
      if (current_progress < total_progress) {
        show_progress_bar(total_progress, total_progress, display_message);
      }
    } else {
      printf("\r\033[K");
      fflush(stdout);
    }
  }

  int result = pclose(fp);

  return result;
}

int execute_command_native(const char *command) { return system(command); }

void parse_and_show_upgrade_result(const char *output, int exit_code) {
  if (config.json_output) {
    printf(
        "{\"operation\": \"upgrade\", \"exit_code\": %d, \"output\": \"%s\"}\n",
        exit_code, output ? output : "");
    return;
  }
  if (exit_code == 0) {
    if (strstr(output, "there is nothing to do") ||
        strstr(output, "nothing to upgrade") || strstr(output, "up to date")) {
      printf("\r\033[32m✓\033[0m %s - System is already up to date\n",
             "Upgrading system");
    } else if (strstr(output, "upgraded") || strstr(output, "installed") ||
               strstr(output, "removed")) {
      int packages = 0;
      char *line = strtok((char *)output, "\n");
      while (line != NULL) {
        if (strstr(line, "upgraded") || strstr(line, "installed") ||
            strstr(line, "removed")) {
          packages++;
        }
        line = strtok(NULL, "\n");
      }
      printf("\r\033[32m✓\033[0m %s - %d packages updated\n",
             "Upgrading system", packages);
    } else {
      printf("\r\033[32m✓\033[0m %s completed successfully\n",
             "Upgrading system");
    }
  } else {
    printf("\r\033[31m✗\033[0m %s failed\n", "Upgrading system");
  }
}

void parse_and_show_install_result(const char *output, int exit_code,
                                   const char *package) {
  if (config.json_output) {
    printf(
        "{\"operation\": \"install\", \"package\": \"%s\", \"exit_code\": %d, "
        "\"output\": \"%s\"}\n",
        package ? package : "", exit_code, output ? output : "");
    return;
  }
  if (exit_code == 0) {
    if (strstr(output, "already installed") || strstr(output, "up to date")) {
      printf("\r\033[33m◦\033[0m Package '%s' is already installed\n", package);
    } else {
      printf("\r\033[32m✓\033[0m Package '%s' installed successfully\n",
             package);
    }
  } else {
    printf("\r\033[31m✗\033[0m Failed to install package '%s'\n", package);
  }
}

void parse_and_show_remove_result(const char *output, int exit_code,
                                  const char *package) {
  if (config.json_output) {
    printf(
        "{\"operation\": \"remove\", \"package\": \"%s\", \"exit_code\": %d, "
        "\"output\": \"%s\"}\n",
        package ? package : "", exit_code, output ? output : "");
    return;
  }
  if (exit_code == 0) {
    if (strstr(output, "not found") || strstr(output, "not installed")) {
      printf("\r\033[33m◦\033[0m Package '%s' was not installed\n", package);
    } else {
      printf("\r\033[32m✓\033[0m Package '%s' removed successfully\n", package);
    }
  } else {
    printf("\r\033[31m✗\033[0m Failed to remove package '%s'\n", package);
  }
}

void parse_and_show_generic_result(const char *output __attribute__((unused)),
                                   int exit_code, const char *operation) {
  if (config.json_output) {
    printf("{\"operation\": \"%s\", \"exit_code\": %d}\n", operation,
           exit_code);
    return;
  }
  if (exit_code == 0) {
    printf("\r\033[32m✓\033[0m %s completed successfully\n", operation);
  } else {
    printf("\r\033[31m✗\033[0m %s failed\n", operation);
  }
}

void get_input(char *input, size_t input_size, const char *prompt) {
  if (!input || input_size == 0) {
    return;
  }

  while (1) {
    char *line = readline(prompt);
    if (line) {
      snprintf(input, input_size, "%s", line);
      free(line);
    } else {
      input[0] = '\0';
      return;
    }

    char *start = input;
    while (isspace((unsigned char)*start)) start++;
    memmove(input, start, strlen(start) + 1);
    char *end = input + strlen(input) - 1;
    while (end > input && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    if (strlen(input) > 0) {
      break;
    }
  }
}

int is_valid_command(const char *command) {
  const char *valid_commands[] = {
      "u",  "i",  "r",  "d",      "p",      "c",    "o",  "s",  "h",
      "q",  "l",  "?",  "cu",     "dt",     "cc",   "lo", "si", "re",
      "ex", "ow", "ba", "health", "config", "help", "pl", "pd", "pe"};
  int num_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);

  if (!command) {
    return 0;
  }

  while (*command == ' ') {
    command++;
  }

  char token[MAX_INPUT_LENGTH];
  const char *space = strchr(command, ' ');
  size_t token_len = space ? (size_t)(space - command) : strlen(command);
  if (token_len >= sizeof(token)) {
    token_len = sizeof(token) - 1;
  }
  memcpy(token, command, token_len);
  token[token_len] = '\0';

  for (int i = 0; i < num_commands; i++) {
    if (strcmp(token, valid_commands[i]) == 0) {
      return 1;
    }
  }

  if (strncmp(command, "u ", 2) == 0 || strncmp(command, "h ", 2) == 0 ||
      strncmp(command, "i ", 2) == 0 || strncmp(command, "r ", 2) == 0 ||
      strncmp(command, "d ", 2) == 0 || strncmp(command, "p ", 2) == 0 ||
      strncmp(command, "s ", 2) == 0 || strncmp(command, "? ", 2) == 0 ||
      strncmp(command, "dt ", 3) == 0 || strncmp(command, "ow ", 3) == 0) {
    return 1;
  }

  if (archium_plugin_is_plugin_command(token)) {
    return 1;
  }

  return 0;
}

int sanitize_shell_input(const char *input, char *output, size_t output_size) {
  if (!input || !output || output_size == 0) {
    return 0;
  }

  size_t input_len = strlen(input);
  if (input_len == 0) {
    output[0] = '\0';
    return 1;
  }

  size_t out_pos = 0;
  for (size_t i = 0; i < input_len && out_pos < output_size - 1; i++) {
    char c = input[i];

    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '+' ||
        c == ':') {
      output[out_pos++] = c;
    } else if (c == ' ' && out_pos > 0 && out_pos < output_size - 2) {
      output[out_pos++] = ' ';
    } else if (c == '/' && out_pos > 0 && out_pos < output_size - 2) {
      output[out_pos++] = '/';
    } else {
      return 0;
    }
  }

  output[out_pos] = '\0';
  return 1;
}

int validate_package_name(const char *package) {
  if (!package || strlen(package) == 0 || strlen(package) > 64) {
    return 0;
  }

  for (size_t i = 0; package[i]; i++) {
    char c = package[i];
    if (!isalnum(c) && c != '-' && c != '_' && c != '.' && c != '+') {
      return 0;
    }
  }

  if (package[0] == '-' || package[0] == '.' || package[0] == '+') {
    return 0;
  }

  return 1;
}

int validate_file_path(const char *path) {
  if (!path || strlen(path) == 0 || strlen(path) > 4096) {
    return 0;
  }

  if (strstr(path, "..") != NULL) {
    return 0;
  }

  if (path[0] == '/') {
    return 0;
  }

  for (size_t i = 0; path[i]; i++) {
    char c = path[i];
    if (!isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' &&
        c != '+') {
      return 0;
    }
  }

  return 1;
}
