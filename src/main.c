#include "archium.h"

char **cached_commands = NULL;

int main(int argc, char *argv[]) {
  ArchiumError status = parse_arguments(argc, argv);
  if (status != ARCHIUM_SUCCESS) {
    return status;
  }

  if (!archium_config_init()) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to initialize configuration system", NULL);
    return ARCHIUM_ERROR_SYSTEM_CALL;
  }

  archium_config_migrate_legacy_files();

  if (!archium_plugin_init()) {
    log_debug("Failed to initialize plugin system");
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  signal(SIGABRT, handle_signal);

  atexit(cleanup_cached_commands);

  log_info("Archium started");

  if (config.version) {
    display_version();
    return ARCHIUM_SUCCESS;
  }

  if (geteuid() != 0 && access("/usr/bin/sudo", X_OK) != 0) {
    archium_report_error(ARCHIUM_ERROR_PERMISSION,
                         "This operation requires root privileges.", NULL);
    return ARCHIUM_ERROR_PERMISSION;
  }

  const char *package_manager;
  int pm_check = check_package_manager();

  switch (pm_check) {
    case 1:
      package_manager = "yay";
      log_info("Using package manager: yay");
      break;
    case 2:
      package_manager = "paru";
      log_info("Using package manager: paru");
      break;
    case 3:
      package_manager = "pacman";
      log_info("Using package manager: pacman");
      break;
    default:
      archium_report_error(ARCHIUM_ERROR_PACKAGE_MANAGER,
                           "No supported package manager found", NULL);
      prompt_install_yay();
      return ARCHIUM_ERROR_PACKAGE_MANAGER;
  }

  rl_attempted_completion_function = command_completion;
  cache_pacman_commands();

  if (config.exec_mode) {
    status = handle_exec_command(config.exec_command, package_manager);
    log_info("Executed command in exec mode");
    cleanup_cached_commands();
    archium_plugin_cleanup();
    return status;
  }

  printf("\033[1;36m╔════════════════════════════════════════╗\033[0m\n");
  printf("\033[1;36m║      Welcome to Archium v%s         ║\033[0m\n",
         ARCHIUM_VERSION);
  printf("\033[1;36m║      Type \"h\" for help                 ║\033[0m\n");
  printf("\033[1;36m╚════════════════════════════════════════╝\033[0m\n");

  display_random_tip();

  while (1) {
    char input_line[MAX_INPUT_LENGTH];
    get_input(input_line, "\033[1;32mArchium $ \033[0m");

    if (*input_line) {
      add_history(input_line);
      status = handle_command(input_line, package_manager);
      if (status != ARCHIUM_SUCCESS) {
        archium_report_error(status, "Command execution failed", input_line);
      }
    }
  }

  return ARCHIUM_SUCCESS;
}
