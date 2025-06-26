#include <time.h>

#include "archium.h"

ArchiumConfig config = {0};

static const char *error_messages[] = {"Success",
                                       "Invalid input",
                                       "System call failed",
                                       "Package manager error",
                                       "Permission denied",
                                       "Operation timed out"};

const char *get_error_string(ArchiumError error_code) {
  size_t index = (error_code < 0) ? (size_t)(-error_code) : (size_t)error_code;
  if (index >= sizeof(error_messages) / sizeof(error_messages[0])) {
    return "Unknown error";
  }
  return error_messages[index];
}

void handle_error(ArchiumError error_code, const char *context) {
  archium_report_error(error_code, context, NULL);
}

static void write_log(const char *level, const char *message) {
  archium_config_write_log(level, message);
}

void log_error(const char *error_message, ArchiumError error_code) {
  archium_report_error(error_code, error_message, NULL);
}

void log_debug(const char *debug_message) { write_log("DEBUG", debug_message); }

void log_info(const char *info_message) { write_log("INFO", info_message); }

void log_action(const char *action) { write_log("ACTION", action); }

ArchiumError parse_arguments(int argc, char *argv[]) {
  config.verbose = 0;
  config.version = 0;
  config.exec_mode = 0;
  config.exec_command = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-V") == 0) {
      config.verbose = 1;
    } else if (strcmp(argv[i], "--version") == 0 ||
               strcmp(argv[i], "-v") == 0) {
      config.version = 1;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      display_cli_help();
      exit(ARCHIUM_SUCCESS);
    } else if (strcmp(argv[i], "--exec") == 0) {
      config.exec_mode = 1;
      if (i + 1 < argc) {
        config.exec_command = argv[++i];
      }
    } else if (strcmp(argv[i], "--self-update") == 0) {
      perform_self_update();
      exit(ARCHIUM_SUCCESS);
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "\033[1;31mError: Unknown option: %s\033[0m\n", argv[i]);
      fprintf(stderr,
              "Use \033[1;32marchium --help\033[0m for usage information.\n");
      return ARCHIUM_ERROR_INVALID_INPUT;
    }
  }

  return ARCHIUM_SUCCESS;
}

void archium_report_error(ArchiumError error_code, const char *context,
                          const char *input) {
  if (error_code == ARCHIUM_SUCCESS) return;
  if (config.verbose) {
    if (input && strlen(input) > 0) {
      fprintf(stderr,
              "\033[1;31m[Archium Error]\033[0m %s: '%s' (code: %d - %s)\n",
              context, input, error_code, get_error_string(error_code));
    } else {
      fprintf(stderr, "\033[1;31m[Archium Error]\033[0m %s (code: %d - %s)\n",
              context, error_code, get_error_string(error_code));
    }
  } else {
    if (error_code == ARCHIUM_ERROR_INVALID_INPUT) {
      printf("\033[1;31mInvalid command. Type 'h' for help.\033[0m\n");
    } else {
      printf("\033[1;31m%s\033[0m\n", get_error_string(error_code));
    }
  }
}
