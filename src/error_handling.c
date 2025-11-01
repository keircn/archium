#include <errno.h>
#include <stdarg.h>
#include <time.h>

#include "include/archium.h"

ArchiumConfig config = {0};

typedef struct {
  ArchiumError code;
  const char *message;
} ErrorMap;

static const ErrorMap error_mappings[] = {
    {ARCHIUM_SUCCESS, "Success"},
    {ARCHIUM_ERROR_INVALID_INPUT, "Invalid input provided"},
    {ARCHIUM_ERROR_INVALID_COMMAND, "Invalid command specified"},
    {ARCHIUM_ERROR_INVALID_ARGUMENT, "Invalid argument provided"},
    {ARCHIUM_ERROR_BUFFER_OVERFLOW, "Buffer overflow detected"},
    {ARCHIUM_ERROR_SYSTEM_CALL, "System call failed"},
    {ARCHIUM_ERROR_FILE_NOT_FOUND, "File not found"},
    {ARCHIUM_ERROR_FILE_ACCESS, "File access error"},
    {ARCHIUM_ERROR_MEMORY_ALLOCATION, "Memory allocation failed"},
    {ARCHIUM_ERROR_PROCESS_FAILED, "Process execution failed"},
    {ARCHIUM_ERROR_PACKAGE_MANAGER, "Package manager error"},
    {ARCHIUM_ERROR_PACKAGE_NOT_FOUND, "Package not found"},
    {ARCHIUM_ERROR_PACKAGE_INSTALL_FAILED, "Package installation failed"},
    {ARCHIUM_ERROR_PACKAGE_REMOVE_FAILED, "Package removal failed"},
    {ARCHIUM_ERROR_PACKAGE_UPDATE_FAILED, "Package update failed"},
    {ARCHIUM_ERROR_PACKAGE_DEPENDENCY, "Package dependency error"},
    {ARCHIUM_ERROR_NETWORK, "Network error"},
    {ARCHIUM_ERROR_DOWNLOAD_FAILED, "Download failed"},
    {ARCHIUM_ERROR_CONNECTION_TIMEOUT, "Connection timeout"},
    {ARCHIUM_ERROR_PERMISSION, "Permission denied"},
    {ARCHIUM_ERROR_PRIVILEGE_REQUIRED, "Elevated privileges required"},
    {ARCHIUM_ERROR_ACCESS_DENIED, "Access denied"},
    {ARCHIUM_ERROR_CONFIG, "Configuration error"},
    {ARCHIUM_ERROR_CONFIG_INVALID, "Invalid configuration"},
    {ARCHIUM_ERROR_CONFIG_MISSING, "Missing configuration"},
    {ARCHIUM_ERROR_PLUGIN, "Plugin error"},
    {ARCHIUM_ERROR_PLUGIN_LOAD_FAILED, "Plugin load failed"},
    {ARCHIUM_ERROR_PLUGIN_INVALID, "Invalid plugin"},
    {ARCHIUM_ERROR_TIMEOUT, "Operation timed out"},
    {ARCHIUM_ERROR_UNKNOWN, "Unknown error"}};

const char *get_error_string(ArchiumError error_code) {
  size_t num_errors = sizeof(error_mappings) / sizeof(error_mappings[0]);
  for (size_t i = 0; i < num_errors; i++) {
    if (error_mappings[i].code == error_code) {
      return error_mappings[i].message;
    }
  }
  return "Unknown error";
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

ArchiumErrorContext *archium_create_error_context(ArchiumError code,
                                                  const char *context,
                                                  const char *function,
                                                  const char *file, int line) {
  static ArchiumErrorContext ctx;

  ctx.error_code = code;
  ctx.timestamp = time(NULL);
  ctx.system_errno = errno;
  ctx.retry_count = 0;
  ctx.line = line;

  strncpy(ctx.context, context ? context : "Unknown context",
          ERROR_CONTEXT_MAX_LEN - 1);
  ctx.context[ERROR_CONTEXT_MAX_LEN - 1] = '\0';

  strncpy(ctx.function, function ? function : "unknown",
          sizeof(ctx.function) - 1);
  ctx.function[sizeof(ctx.function) - 1] = '\0';

  strncpy(ctx.file, file ? file : "unknown", sizeof(ctx.file) - 1);
  ctx.file[sizeof(ctx.file) - 1] = '\0';

  ctx.details[0] = '\0';

  return &ctx;
}

void archium_error_add_details(ArchiumErrorContext *ctx, const char *details,
                               ...) {
  if (!ctx || !details) return;

  va_list args;
  va_start(args, details);
  vsnprintf(ctx->details, ERROR_DETAILS_MAX_LEN - 1, details, args);
  ctx->details[ERROR_DETAILS_MAX_LEN - 1] = '\0';
  va_end(args);
}

const char *archium_error_get_category(ArchiumError error_code) {
  if (error_code >= ARCHIUM_ERROR_INVALID_INPUT &&
      error_code >= ARCHIUM_ERROR_BUFFER_OVERFLOW) {
    return "Input/Validation";
  } else if (error_code >= ARCHIUM_ERROR_SYSTEM_CALL &&
             error_code >= ARCHIUM_ERROR_PROCESS_FAILED) {
    return "System";
  } else if (error_code >= ARCHIUM_ERROR_PACKAGE_MANAGER &&
             error_code >= ARCHIUM_ERROR_PACKAGE_DEPENDENCY) {
    return "Package Management";
  } else if (error_code >= ARCHIUM_ERROR_NETWORK &&
             error_code >= ARCHIUM_ERROR_CONNECTION_TIMEOUT) {
    return "Network";
  } else if (error_code >= ARCHIUM_ERROR_PERMISSION &&
             error_code >= ARCHIUM_ERROR_ACCESS_DENIED) {
    return "Permission/Security";
  } else if (error_code >= ARCHIUM_ERROR_CONFIG &&
             error_code >= ARCHIUM_ERROR_CONFIG_MISSING) {
    return "Configuration";
  } else if (error_code >= ARCHIUM_ERROR_PLUGIN &&
             error_code >= ARCHIUM_ERROR_PLUGIN_INVALID) {
    return "Plugin";
  }
  return "General";
}

int archium_error_is_recoverable(ArchiumError error_code) {
  switch (error_code) {
    case ARCHIUM_ERROR_NETWORK:
    case ARCHIUM_ERROR_DOWNLOAD_FAILED:
    case ARCHIUM_ERROR_CONNECTION_TIMEOUT:
    case ARCHIUM_ERROR_TIMEOUT:
    case ARCHIUM_ERROR_PACKAGE_UPDATE_FAILED:
    case ARCHIUM_ERROR_PACKAGE_INSTALL_FAILED:
      return 1;
    case ARCHIUM_ERROR_PERMISSION:
    case ARCHIUM_ERROR_ACCESS_DENIED:
    case ARCHIUM_ERROR_MEMORY_ALLOCATION:
    case ARCHIUM_ERROR_CONFIG_MISSING:
      return 0;
    default:
      return 0;
  }
}

void archium_report_error_detailed(const ArchiumErrorContext *error_ctx) {
  if (!error_ctx) return;

  char time_str[64];
  struct tm *tm_info = localtime(&error_ctx->timestamp);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

  char log_message[2048];
  snprintf(log_message, sizeof(log_message),
           "ERROR [%s] %s:%d in %s() - %s (code: %d) | %s | Category: %s | "
           "Recoverable: %s | Errno: %d%s%s",
           time_str, error_ctx->file, error_ctx->line, error_ctx->function,
           error_ctx->context, error_ctx->error_code,
           get_error_string(error_ctx->error_code),
           archium_error_get_category(error_ctx->error_code),
           archium_error_is_recoverable(error_ctx->error_code) ? "Yes" : "No",
           error_ctx->system_errno, error_ctx->details[0] ? " | Details: " : "",
           error_ctx->details[0] ? error_ctx->details : "");

  archium_config_write_log("ERROR", log_message);

  if (config.verbose) {
    fprintf(stderr, "\033[1;31m[Archium Error]\033[0m %s\n", log_message);
  } else {
    fprintf(stderr, "\033[1;31mError:\033[0m %s\n",
            get_error_string(error_ctx->error_code));
    if (error_ctx->details[0]) {
      fprintf(stderr, "\033[1;33mDetails:\033[0m %s\n", error_ctx->details);
    }
    if (archium_error_is_recoverable(error_ctx->error_code)) {
      fprintf(
          stderr,
          "\033[1;32mTip:\033[0m This error may be temporary. Try again.\n");
    }

    archium_suggest_recovery_action(error_ctx->error_code);
  }
}

ArchiumError archium_retry_operation(ArchiumRetryFunc func, void *user_data,
                                     int max_retries, int delay_ms,
                                     const char *operation_name) {
  if (!func) return ARCHIUM_ERROR_INVALID_ARGUMENT;

  ArchiumError result = ARCHIUM_ERROR_UNKNOWN;

  for (int attempt = 1; attempt <= max_retries; attempt++) {
    result = func(user_data);

    if (result == ARCHIUM_SUCCESS) {
      return ARCHIUM_SUCCESS;
    }

    if (!archium_error_is_recoverable(result)) {
      log_debug("Operation not recoverable, stopping retries");
      break;
    }

    if (attempt < max_retries) {
      log_info("Retrying operation after delay...");
      if (delay_ms > 0) {
        usleep(delay_ms * 1000);
      }
    }
  }

  char retry_context[256];
  snprintf(retry_context, sizeof(retry_context),
           "Operation '%s' failed after %d attempts",
           operation_name ? operation_name : "unknown", max_retries);

  ArchiumErrorContext *ctx = archium_create_error_context(
      result, retry_context, __func__, __FILE__, __LINE__);
  archium_error_add_details(ctx, "Max retries (%d) exceeded", max_retries);
  archium_report_error_detailed(ctx);

  return result;
}

void archium_suggest_recovery_action(ArchiumError error_code) {
  switch (error_code) {
    case ARCHIUM_ERROR_PERMISSION:
    case ARCHIUM_ERROR_PRIVILEGE_REQUIRED:
      printf(
          "\033[1;36mSuggestion:\033[0m Try running with sudo or check file "
          "permissions.\n");
      break;

    case ARCHIUM_ERROR_NETWORK:
    case ARCHIUM_ERROR_CONNECTION_TIMEOUT:
      printf(
          "\033[1;36mSuggestion:\033[0m Check your internet connection and try "
          "again.\n");
      break;

    case ARCHIUM_ERROR_PACKAGE_NOT_FOUND:
      printf(
          "\033[1;36mSuggestion:\033[0m Update package databases with 'u' "
          "command first.\n");
      break;

    case ARCHIUM_ERROR_PACKAGE_DEPENDENCY:
      printf(
          "\033[1;36mSuggestion:\033[0m Try installing dependencies manually "
          "or use --force flag.\n");
      break;

    case ARCHIUM_ERROR_FILE_NOT_FOUND:
      printf(
          "\033[1;36mSuggestion:\033[0m Check if the file path exists and is "
          "accessible.\n");
      break;

    case ARCHIUM_ERROR_MEMORY_ALLOCATION:
      printf(
          "\033[1;36mSuggestion:\033[0m Close other applications to free "
          "memory and restart.\n");
      break;

    case ARCHIUM_ERROR_CONFIG_MISSING:
      printf(
          "\033[1;36mSuggestion:\033[0m Run archium setup to initialize "
          "configuration.\n");
      break;

    case ARCHIUM_ERROR_PACKAGE_MANAGER:
      printf(
          "\033[1;36mSuggestion:\033[0m Install yay or paru package manager "
          "first.\n");
      break;

    default:
      break;
  }
}
