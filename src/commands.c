#include "archium.h"

static void execute_command(const char *command, const char *log_message) {
  int ret = system(command);
  if (ret != 0) {
    fprintf(stderr, "\033[1;31mError: Command failed: %s\033[0m\n", command);
    if (config.verbose) {
      printf("\033[1;31m[Status]\033[0m Command failed (exit code: %d)\n", ret);
    }
  } else {
    if (config.verbose) {
      printf("\033[1;32m[Status]\033[0m Command succeeded\n");
    }
  }
  if (log_message) {
    log_action(log_message);
  }
}

static void get_user_input(char *buffer, const char *prompt) {
  rl_attempted_completion_function = command_completion;
  get_input(buffer, prompt);
  rl_attempted_completion_function = NULL;
}

ArchiumError handle_command(const char *input, const char *package_manager) {
  log_action(input);

  if (!is_valid_command(input)) {
    return ARCHIUM_ERROR_INVALID_INPUT;
  }

  if (strcmp(input, "h") == 0 || strcmp(input, "help") == 0) {
    display_help();
    return ARCHIUM_SUCCESS;
  }

  if (strncmp(input, "h ", 2) == 0) {
    const char *help_arg = input + 2;
    if (strcmp(help_arg, "quick") == 0) {
      display_help_quick();
    } else if (strcmp(help_arg, "packages") == 0 ||
               strcmp(help_arg, "system") == 0 ||
               strcmp(help_arg, "info") == 0 ||
               strcmp(help_arg, "config") == 0) {
      display_help_category(help_arg);
    } else {
      display_help_command(help_arg);
    }
    return ARCHIUM_SUCCESS;
  }

  if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0 ||
      strcmp(input, "exit") == 0) {
    printf("Exiting Archium.\n");
    archium_plugin_cleanup();
    exit(0);
  }

  if (strcmp(input, "u") == 0) {
    update_system(package_manager, NULL);
  } else if (strncmp(input, "u ", 2) == 0) {
    update_system(package_manager, input + 2);
  } else if (strcmp(input, "i") == 0) {
    char packages[MAX_INPUT_LENGTH];
    get_user_input(packages, "Enter package names to install: ");
    install_package(package_manager, packages);
  } else if (strcmp(input, "r") == 0) {
    char packages[MAX_INPUT_LENGTH];
    get_user_input(packages, "Enter package names to remove: ");
    remove_package(package_manager, packages);
  } else if (strcmp(input, "p") == 0) {
    char packages[MAX_INPUT_LENGTH];
    get_user_input(packages, "Enter package names to purge: ");
    purge_package(package_manager, packages);
  } else if (strcmp(input, "c") == 0) {
    clean_cache(package_manager);
  } else if (strcmp(input, "cc") == 0) {
    clear_build_cache();
  } else if (strcmp(input, "o") == 0) {
    clean_orphans(package_manager);
  } else if (strcmp(input, "lo") == 0) {
    list_orphans();
  } else if (strcmp(input, "s") == 0) {
    char package[MAX_INPUT_LENGTH];
    get_user_input(package, "Enter package name to search: ");
    search_package(package_manager, package);
  } else if (strcmp(input, "l") == 0) {
    list_installed_packages();
  } else if (strcmp(input, "?") == 0) {
    char package[MAX_INPUT_LENGTH];
    get_user_input(package, "Enter package name to show info: ");
    show_package_info(package_manager, package);
  } else if (strcmp(input, "cu") == 0) {
    check_package_updates();
  } else if (strcmp(input, "dt") == 0) {
    char package[MAX_INPUT_LENGTH];
    get_user_input(package, "Enter package name to view dependencies: ");
    display_dependency_tree(package_manager, package);
  } else if (strcmp(input, "si") == 0) {
    list_packages_by_size();
  } else if (strcmp(input, "re") == 0) {
    list_recent_installs();
  } else if (strcmp(input, "ex") == 0) {
    list_explicit_installs();
  } else if (strcmp(input, "ow") == 0) {
    char file[MAX_INPUT_LENGTH];
    get_user_input(file, "Enter file path: ");
    find_package_owner(file);
  } else if (strcmp(input, "ba") == 0) {
    backup_pacman_config();
  } else if (strcmp(input, "config") == 0) {
    configure_preferences();
  } else if (strcmp(input, "plugin") == 0 || strcmp(input, "plugins") == 0) {
    manage_plugins();
  } else if (archium_plugin_is_plugin_command(input)) {
    return archium_plugin_execute(input, "", package_manager);
  }

  return ARCHIUM_SUCCESS;
}

ArchiumError handle_exec_command(const char *command,
                                 const char *package_manager) {
  if (!command || !package_manager) {
    return ARCHIUM_ERROR_INVALID_INPUT;
  }

  log_action(command);

  return handle_command(command, package_manager);
}

void update_system(const char *package_manager, const char *package) {
  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[4096];

  if (package) {
    snprintf(command, sizeof(command), "%s -S %s", package_manager, package);
    int result = execute_command_with_output_capture(
        command, "Upgrading package", output_buffer, sizeof(output_buffer));
    parse_and_show_install_result(output_buffer, result, package);
  } else {
    snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
    int result = execute_command_with_output_capture(
        command, "Upgrading system", output_buffer, sizeof(output_buffer));
    parse_and_show_upgrade_result(output_buffer, result);
  }
}

void clear_build_cache() {
  const char *cache_dir = archium_config_get_cache_dir();
  char output_buffer[1024];

  if (cache_dir) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "rm -rf %s/*", cache_dir);
    int result = execute_command_with_output_capture(
        command, "Clearing Archium cache", output_buffer,
        sizeof(output_buffer));
    parse_and_show_generic_result(output_buffer, result,
                                  "Clearing Archium cache");
  }

  int result1 = execute_command_with_output_capture(
      "rm -rf $HOME/.cache/yay", "Clearing yay cache", output_buffer,
      sizeof(output_buffer));
  parse_and_show_generic_result(output_buffer, result1, "Clearing yay cache");

  int result2 = execute_command_with_output_capture(
      "rm -rf $HOME/.cache/paru", "Clearing paru cache", output_buffer,
      sizeof(output_buffer));
  parse_and_show_generic_result(output_buffer, result2, "Clearing paru cache");
}

void list_orphans() {
  printf("\033[1;34mListing orphaned packages...\033[0m\n");
  execute_command("pacman -Qdt", NULL);
}

void install_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[4096];
  snprintf(command, sizeof(command), "%s -S %s", package_manager, packages);
  int result = execute_command_with_output_capture(
      command, "Installing packages", output_buffer, sizeof(output_buffer));
  parse_and_show_install_result(output_buffer, result, packages);
}

void remove_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[4096];
  snprintf(command, sizeof(command), "%s -R %s", package_manager, packages);
  int result = execute_command_with_output_capture(
      command, "Removing packages", output_buffer, sizeof(output_buffer));
  parse_and_show_remove_result(output_buffer, result, packages);
}

void purge_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[4096];
  snprintf(command, sizeof(command), "%s -Rns %s", package_manager, packages);
  int result = execute_command_with_output_capture(
      command, "Purging packages", output_buffer, sizeof(output_buffer));
  parse_and_show_remove_result(output_buffer, result, packages);
}

void clean_cache(const char *package_manager) {
  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[2048];
  snprintf(command, sizeof(command), "%s -Sc --noconfirm", package_manager);
  int result = execute_command_with_output_capture(
      command, "Cleaning package cache", output_buffer, sizeof(output_buffer));
  parse_and_show_generic_result(output_buffer, result,
                                "Cleaning package cache");
}

void clean_orphans(const char *package_manager) {
  char orphan_check_command[COMMAND_BUFFER_SIZE];
  snprintf(orphan_check_command, sizeof(orphan_check_command), "pacman -Qdtq");

  FILE *fp = popen(orphan_check_command, "r");
  if (!fp) {
    fprintf(stderr,
            "\033[1;31mError: Failed to check for orphaned packages.\033[0m\n");
    return;
  }

  char orphaned_packages[COMMAND_BUFFER_SIZE] = {0};
  if (!fgets(orphaned_packages, sizeof(orphaned_packages), fp)) {
    pclose(fp);
    printf("\033[1;32mNo orphaned packages found.\033[0m\n");
    return;
  }
  pclose(fp);

  orphaned_packages[strcspn(orphaned_packages, "\n")] = '\0';

  char command[COMMAND_BUFFER_SIZE];
  char output_buffer[2048];
  snprintf(command, sizeof(command), "%s -Rns %s", package_manager,
           orphaned_packages);
  int result =
      execute_command_with_output_capture(command, "Cleaning orphaned packages",
                                          output_buffer, sizeof(output_buffer));
  parse_and_show_generic_result(output_buffer, result,
                                "Cleaning orphaned packages");
}

void search_package(const char *package_manager, const char *package) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -Ss %s", package_manager, package);
  printf("\033[1;34mSearching for package: %s\033[0m\n", package);
  execute_command(command, NULL);
}

void list_installed_packages(void) {
  printf("\033[1;34mListing installed packages...\033[0m\n");
  execute_command("pacman -Qe", NULL);
}

void show_package_info(const char *package_manager, const char *package) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -Si %s", package_manager, package);
  printf("\033[1;34mShowing information for package: %s\033[0m\n", package);
  execute_command(command, NULL);
}

void check_package_updates(void) {
  printf("\033[1;34mChecking for package updates...\033[0m\n");
  execute_command("pacman -Qu", "Checked for updates");
}

void display_dependency_tree(const char *package_manager, const char *package) {
  (void)package_manager;
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "pactree %s", package);
  printf("\033[1;34mDisplaying dependency tree for package: %s\033[0m\n",
         package);
  execute_command(command, NULL);
}

void perform_self_update(void) {
  FILE *fp = popen("pacman -Qm | grep '^archium '", "r");
  if (!fp) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to check installation source", NULL);
    return;
  }

  char buffer[COMMAND_BUFFER_SIZE];
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    pclose(fp);
    printf("\033[1;33mWarning: Archium appears to be installed via AUR.\n");
    printf("Please use your AUR helper to update instead:\n");
    printf("yay -Syu archium\n");
    printf("or\n");
    printf("paru -Syu archium\033[0m\n");
    return;
  }
  pclose(fp);

  char clone_dir[COMMAND_BUFFER_SIZE];
  char command[COMMAND_BUFFER_SIZE];
  int ret;

  const char *cache_dir = archium_config_get_cache_dir();
  if (!cache_dir) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to get cache directory", NULL);
    return;
  }

  ret = snprintf(clone_dir, sizeof(clone_dir), "%s/archium-update-%d",
                 cache_dir, (int)time(NULL));
  if (ret >= (int)sizeof(clone_dir)) {
    archium_report_error(ARCHIUM_ERROR_INVALID_INPUT,
                         "Clone directory path too long", NULL);
    return;
  }

  log_info("Starting self-update process");

  if (mkdir(clone_dir, 0755) != 0) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create temporary directory", NULL);
    return;
  }

  ret = snprintf(command, sizeof(command), "git clone %.256s %.256s",
                 ARCHIUM_REPO_URL, clone_dir);
  if (ret >= (int)sizeof(command)) {
    archium_report_error(ARCHIUM_ERROR_INVALID_INPUT, "Command string too long",
                         NULL);
    rmdir(clone_dir);
    return;
  }

  char output_buffer[2048];
  int result1 =
      execute_command_with_output_capture(command, "Cloning Archium repository",
                                          output_buffer, sizeof(output_buffer));
  if (result1 != 0) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to clone repository", NULL);
    rmdir(clone_dir);
    return;
  }
  parse_and_show_generic_result(output_buffer, result1,
                                "Cloning Archium repository");

  if (chdir(clone_dir) != 0) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to change to clone directory", NULL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }

  int result2 = execute_command_with_output_capture(
      "make", "Building Archium", output_buffer, sizeof(output_buffer));
  if (result2 != 0) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL, "Failed to build project",
                         NULL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }
  parse_and_show_generic_result(output_buffer, result2, "Building Archium");

  int result3 = execute_command_with_output_capture(
      "sudo make install", "Installing updates", output_buffer,
      sizeof(output_buffer));
  if (result3 != 0) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL, "Failed to install updates",
                         NULL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }
  parse_and_show_generic_result(output_buffer, result3, "Installing updates");

  ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
  if (ret < (int)sizeof(command)) {
    system(command);
  }

  printf("\033[1;32mArchium has been updated successfully!\033[0m\n");
  log_info("Self-update completed successfully");
}

void list_packages_by_size(void) {
  printf("\033[1;34mListing installed packages by size...\033[0m\n");
  execute_command(
      "pacman -Qi | awk '/^Name/{name=$3} /^Installed Size/{size=$4$5; print "
      "size, name}' | sort -h",
      "Listed packages by size");
}

void list_recent_installs(void) {
  printf("\033[1;34mListing recently installed packages...\033[0m\n");
  execute_command("grep -i installed /var/log/pacman.log | tail -n 20",
                  "Listed recent installations");
}

void list_explicit_installs(void) {
  printf("\033[1;34mListing explicitly installed packages...\033[0m\n");
  execute_command(
      "pacman -Qei | awk '/^Name/ { name=$3 } /^Groups/ { if ($3 != \"base\" "
      "&& $3 != \"base-devel\") { print name } }'",
      "Listed explicit installations");
}

void find_package_owner(const char *file) {
  struct stat st;
  if (stat(file, &st) != 0) {
    fprintf(stderr,
            "\033[1;31mError: File '%s' does not exist or cannot be "
            "accessed.\033[0m\n",
            file);
    return;
  }

  if (access(file, R_OK) != 0) {
    fprintf(stderr,
            "\033[1;31mError: Insufficient permissions to read '%s'.\033[0m\n",
            file);
    return;
  }

  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "pacman -Qo %s", file);
  printf("\033[1;34mFinding package owner for: %s\033[0m\n", file);
  execute_command(command, NULL);
}

void backup_pacman_config(void) {
  time_t now = time(NULL);
  char timestamp[32];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));

  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command),
           "sudo cp /etc/pacman.conf /etc/pacman.conf.backup_%s", timestamp);

  printf("\033[1;34mBacking up pacman configuration...\033[0m\n");
  if (system(command) == 0) {
    printf("\033[1;32mBackup created: /etc/pacman.conf.backup_%s\033[0m\n",
           timestamp);
    log_info("Pacman configuration backed up");
  } else {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to backup pacman configuration", NULL);
  }
}

void configure_preferences(void) {
  printf("\033[1;34mArchium Configuration\033[0m\n");
  printf("Available preferences:\n");
  printf("1. Package manager preference (yay/paru)\n");
  printf("2. View current configuration directory\n");
  printf("3. View log file location\n");

  char choice[MAX_INPUT_LENGTH];
  get_user_input(choice, "Enter your choice (1-3): ");

  if (strcmp(choice, "1") == 0) {
    printf("\033[1;33mCurrent preference:\033[0m ");
    if (archium_config_check_paru_preference()) {
      printf("paru\n");
    } else {
      printf("yay (default)\n");
    }

    char pref[MAX_INPUT_LENGTH];
    get_user_input(pref, "Set package manager preference (yay/paru): ");

    if (strcmp(pref, "yay") == 0 || strcmp(pref, "paru") == 0) {
      if (archium_config_set_preference("package_manager", pref)) {
        printf("\033[1;32mPackage manager preference set to: %s\033[0m\n",
               pref);
        printf(
            "\033[1;33mNote: Restart Archium for changes to take "
            "effect.\033[0m\n");
        log_action("Package manager preference updated");
      } else {
        printf("\033[1;31mFailed to set preference.\033[0m\n");
      }
    } else {
      printf(
          "\033[1;31mInvalid choice. Please enter 'yay' or 'paru'.\033[0m\n");
    }
  } else if (strcmp(choice, "2") == 0) {
    const char *config_dir = archium_config_get_config_dir();
    if (config_dir) {
      printf("\033[1;32mConfiguration directory: %s\033[0m\n", config_dir);
    } else {
      printf("\033[1;31mFailed to get configuration directory.\033[0m\n");
    }
  } else if (strcmp(choice, "3") == 0) {
    const char *log_file = archium_config_get_log_file();
    if (log_file) {
      printf("\033[1;32mLog file location: %s\033[0m\n", log_file);
    } else {
      printf("\033[1;31mFailed to get log file location.\033[0m\n");
    }
  } else {
    printf("\033[1;31mInvalid choice.\033[0m\n");
  }
}

void manage_plugins(void) {
  printf("\033[1;34mArchium Plugin Management\033[0m\n");
  printf("Available options:\n");
  printf("1. List loaded plugins\n");
  printf("2. View plugin directory\n");
  printf("3. Create example plugin\n");

  char choice[MAX_INPUT_LENGTH];
  get_user_input(choice, "Enter your choice (1-3): ");

  if (strcmp(choice, "1") == 0) {
    archium_plugin_list_loaded();
  } else if (strcmp(choice, "2") == 0) {
    const char *plugin_dir = archium_config_get_plugin_dir();
    if (plugin_dir) {
      printf("\033[1;32mPlugin directory: %s\033[0m\n", plugin_dir);
      printf(
          "\033[1;33mPlace .so files in this directory to load them as "
          "plugins.\033[0m\n");
    } else {
      printf("\033[1;31mFailed to get plugin directory.\033[0m\n");
    }
  } else if (strcmp(choice, "3") == 0) {
    if (archium_plugin_create_example()) {
      const char *plugin_dir = archium_config_get_plugin_dir();
      printf("\033[1;32mExample plugin created successfully!\033[0m\n");
      printf("\033[1;33mLocation: %s/example.c\033[0m\n",
             plugin_dir ? plugin_dir : "unknown");
      printf("\033[1;33mTo build: cd %s && make\033[0m\n",
             plugin_dir ? plugin_dir : "unknown");
      printf("\033[1;33mRestart Archium to load the plugin.\033[0m\n");
      log_action("Example plugin created");
    } else {
      printf("\033[1;31mFailed to create example plugin.\033[0m\n");
    }
  } else {
    printf("\033[1;31mInvalid choice.\033[0m\n");
  }
}