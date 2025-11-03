#include <pthread.h>
#include <sys/ioctl.h>
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

  if (pthread_create(&spinner_tid, NULL, spinner_thread, &spinner_data) != 0) {
    return system(command);
  }

  char silent_command[1024];
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
  int running = 1;
  pthread_t spinner_tid;
  spinner_data_t spinner_data = {&running, message ? message : "Processing"};

  if (pthread_create(&spinner_tid, NULL, spinner_thread, &spinner_data) != 0) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) return -1;

    if (output_buffer && buffer_size > 0) {
      size_t bytes_read = fread(output_buffer, 1, buffer_size - 1, fp);
      output_buffer[bytes_read] = '\0';
    }

    return pclose(fp);
  }

  FILE *fp = popen(command, "r");
  if (fp == NULL) {
    running = 0;
    pthread_join(spinner_tid, NULL);
    return -1;
  }

  if (output_buffer && buffer_size > 0) {
    size_t bytes_read = fread(output_buffer, 1, buffer_size - 1, fp);
    output_buffer[bytes_read] = '\0';
  }

  int result = pclose(fp);

  running = 0;
  pthread_join(spinner_tid, NULL);

  return result;
}

void parse_and_show_upgrade_result(const char *output, int exit_code) {
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
  if (exit_code == 0) {
    printf("\r\033[32m✓\033[0m %s completed successfully\n", operation);
  } else {
    printf("\r\033[31m✗\033[0m %s failed\n", operation);
  }
}

void get_input(char *input, const char *prompt) {
  while (1) {
    char *line = readline(prompt);
    if (line) {
      strcpy(input, line);
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
      "u",  "i",  "r",      "p",    "c",  "o",  "s",  "h",  "q",
      "l",  "?",  "cu",     "dt",   "cc", "lo", "si", "re", "ex",
      "ow", "ba", "config", "help", "pl", "pd", "pe"};
  int num_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);

  for (int i = 0; i < num_commands; i++) {
    if (strcmp(command, valid_commands[i]) == 0) {
      return 1;
    }
  }

  if (strncmp(command, "u ", 2) == 0 || strncmp(command, "h ", 2) == 0) {
    return 1;
  }

  if (archium_plugin_is_plugin_command(command)) {
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