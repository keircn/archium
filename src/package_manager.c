#include "include/archium.h"

int check_archium_file(void) { return archium_config_check_paru_preference(); }

int check_command(const char *command) {
  char cmd[COMMAND_BUFFER_SIZE];
  if (snprintf(cmd, sizeof(cmd), "command -v %s > /dev/null 2>&1", command) >=
      (int)sizeof(cmd)) {
    fprintf(stderr, "\033[1;31mError: Command buffer overflow.\033[0m\n");
    return 0;
  }
  return (system(cmd) == 0);
}

int check_package_manager(void) {
  if (check_archium_file()) {
    return 2;  // paru
  }
  if (check_command("yay")) {
    return 1;  // yay
  }
  if (check_command("paru")) {
    return 2;  // paru
  }
  if (check_command("pacman")) {
    return 3;  // pacman
  }
  return 0;  // none
}

char *get_package_manager_version(const char *package_manager) {
  if (!package_manager || strcmp(package_manager, "none") == 0) {
    return strdup("unknown");
  }

  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s --version | head -n 1",
           package_manager);

  FILE *fp = popen(command, "r");
  if (!fp) {
    fprintf(stderr,
            "\033[1;31mError: Failed to retrieve version for %s\033[0m\n",
            package_manager);
    return strdup("unknown");
  }

  char version[COMMAND_BUFFER_SIZE];
  if (!fgets(version, sizeof(version), fp)) {
    pclose(fp);
    return strdup("unknown");
  }
  pclose(fp);

  version[strcspn(version, "\n")] = '\0';

  return strdup(version);
}

int check_git(void) { return check_command("git"); }

void install_git(void) {
  char output_buffer[2048];
  int result = execute_command_with_output_capture(
      "sudo pacman -S --noconfirm git", "Installing git", output_buffer,
      sizeof(output_buffer));
  if (result != 0) {
    fprintf(stderr, "\033[1;31mError: Failed to install git.\033[0m\n");
    exit(EXIT_FAILURE);
  }
  parse_and_show_install_result(output_buffer, result, "git");
}

void install_yay(void) {
  const char *cache_dir = archium_config_get_cache_dir();
  if (!cache_dir) {
    fprintf(stderr, "\033[1;31mError: Failed to get cache directory.\033[0m\n");
    exit(EXIT_FAILURE);
  }

  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[4096];
  int ret = snprintf(command, sizeof(command),
                     "mkdir -p %s/setup && "
                     "cd %s/setup && "
                     "git clone https://aur.archlinux.org/yay-bin.git && "
                     "cd yay-bin && "
                     "makepkg -scCi && "
                     "cd && "
                     "rm -rf %s/setup",
                     cache_dir, cache_dir, cache_dir);

  if (ret >= (int)sizeof(command)) {
    fprintf(stderr, "\033[1;31mError: Command string too long.\033[0m\n");
    exit(EXIT_FAILURE);
  }

  int result = execute_command_with_output_capture(
      command, "Installing yay", output_buffer, sizeof(output_buffer));
  if (result != 0) {
    fprintf(stderr, "\033[1;31mError: Failed to install yay.\033[0m\n");
    exit(EXIT_FAILURE);
  }
  parse_and_show_install_result(output_buffer, result, "yay");
  printf(
      "\033[1;32mInstallation of yay is complete. Please restart your shell "
      "and relaunch Archium.\033[0m\n");
}

void prompt_install_yay(void) {
  char response[10];
  printf("\033[1;31mError: No suitable package manager is installed.\033[0m\n");
  sleep(1);
  printf(
      "Do you want to install yay? (\033[1;32my\033[0m/\033[1;31mn\033[0m): ");

  if (!fgets(response, sizeof(response), stdin)) {
    fprintf(stderr, "\033[1;31mError: Failed to read input.\033[0m\n");
    exit(EXIT_FAILURE);
  }

  response[strcspn(response, "\n")] = '\0';

  if (strcasecmp(response, "y") == 0 || strcasecmp(response, "yes") == 0) {
    if (!check_git()) {
      install_git();
    }
    install_yay();
  } else {
    printf("\033[1;31mExiting Archium.\033[0m\n");
    exit(EXIT_FAILURE);
  }
}