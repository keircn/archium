#include "archium.h"

static void execute_command(const char *command, const char *log_message) {
  int ret = system(command);
  if (ret != 0) {
    fprintf(stderr, "\033[1;31mError: Command failed: %s\033[0m\n", command);
    printf("\033[1;31m[Status]\033[0m Command failed (exit code: %d)\n", ret);
  } else {
    printf("\033[1;32m[Status]\033[0m Command succeeded\n");
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
    fprintf(stderr, "Error: Invalid command. Type 'h' for help.\n");
    return ARCHIUM_ERROR_INVALID_INPUT;
  }

  if (strcmp(input, "h") == 0 || strcmp(input, "help") == 0) {
    display_help();
    return ARCHIUM_SUCCESS;
  }

  if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0 ||
      strcmp(input, "exit") == 0) {
    printf("Exiting Archium.\n");
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
  if (package) {
    snprintf(command, sizeof(command), "%s -S %s", package_manager, package);
    printf("\033[1;34mUpgrading package: %s\033[0m\n", package);
  } else {
    snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
    printf("\033[1;34mUpgrading system...\033[0m\n");
  }
  execute_command(command, "System updated");
}

void clear_build_cache() {
  printf("\033[1;34mClearing package build cache...\033[0m\n");
  execute_command("rm -rf $HOME/.cache/yay", "Cleared package build cache");
}

void list_orphans() {
  printf("\033[1;34mListing orphaned packages...\033[0m\n");
  execute_command("pacman -Qdt", NULL);
}

void install_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -S %s", package_manager, packages);
  printf("\033[1;34mInstalling packages: %s\033[0m\n", packages);
  execute_command(command, "Installed packages");
}

void remove_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -R %s", package_manager, packages);
  printf("\033[1;34mRemoving packages: %s\033[0m\n", packages);
  execute_command(command, "Removed packages");
}

void purge_package(const char *package_manager, const char *packages) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -Rns %s", package_manager, packages);
  printf("\033[1;34mPurging packages: %s\033[0m\n", packages);
  execute_command(command, "Purged packages");
}

void clean_cache(const char *package_manager) {
  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -Sc --noconfirm", package_manager);
  printf("\033[1;34mCleaning package cache...\033[0m\n");
  execute_command(command, "Cache cleaned");
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
  snprintf(command, sizeof(command), "%s -Rns %s", package_manager,
           orphaned_packages);
  printf("\033[1;34mCleaning orphaned packages...\033[0m\n");
  execute_command(command, "Orphaned packages cleaned");
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
    log_error("Failed to check installation source", ARCHIUM_ERROR_SYSTEM_CALL);
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

  ret = snprintf(clone_dir, sizeof(clone_dir), "/tmp/archium-update-%d",
                 (int)time(NULL));
  if (ret >= (int)sizeof(clone_dir)) {
    log_error("Clone directory path too long", ARCHIUM_ERROR_INVALID_INPUT);
    return;
  }

  printf("\033[1;34mUpdating Archium...\033[0m\n");
  log_info("Starting self-update process");

  if (mkdir(clone_dir, 0755) != 0) {
    log_error("Failed to create temporary directory",
              ARCHIUM_ERROR_SYSTEM_CALL);
    return;
  }

  ret = snprintf(command, sizeof(command), "git clone %.256s %.256s",
                 ARCHIUM_REPO_URL, clone_dir);
  if (ret >= (int)sizeof(command)) {
    log_error("Command string too long", ARCHIUM_ERROR_INVALID_INPUT);
    rmdir(clone_dir);
    return;
  }

  if (system(command) != 0) {
    log_error("Failed to clone repository", ARCHIUM_ERROR_SYSTEM_CALL);
    rmdir(clone_dir);
    return;
  }

  if (chdir(clone_dir) != 0) {
    log_error("Failed to change to clone directory", ARCHIUM_ERROR_SYSTEM_CALL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }

  if (system("make") != 0) {
    log_error("Failed to build project", ARCHIUM_ERROR_SYSTEM_CALL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }

  printf(
      "\033[1;33mRequesting elevated privileges to install "
      "updates...\033[0m\n");
  if (system("sudo make install") != 0) {
    log_error("Failed to install updates", ARCHIUM_ERROR_SYSTEM_CALL);
    ret = snprintf(command, sizeof(command), "rm -rf %.256s", clone_dir);
    if (ret < (int)sizeof(command)) {
      system(command);
    }
    return;
  }

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
    log_error("Failed to backup pacman configuration",
              ARCHIUM_ERROR_SYSTEM_CALL);
  }
}