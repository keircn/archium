#ifndef ERROR_H
#define ERROR_H

#include <time.h>

#define ERROR_CONTEXT_MAX_LEN 512
#define ERROR_DETAILS_MAX_LEN 1024

#define ARCHIUM_ERROR(code, context)                          \
  archium_report_error_detailed(archium_create_error_context( \
      code, context, __func__, __FILE__, __LINE__))

#define ARCHIUM_ERROR_WITH_DETAILS(code, context, ...)        \
  do {                                                        \
    ArchiumErrorContext *_ctx = archium_create_error_context( \
        code, context, __func__, __FILE__, __LINE__);         \
    archium_error_add_details(_ctx, __VA_ARGS__);             \
    archium_report_error_detailed(_ctx);                      \
  } while (0)

typedef enum {
  ARCHIUM_SUCCESS = 0,
  ARCHIUM_ERROR_INVALID_INPUT = -1,
  ARCHIUM_ERROR_INVALID_COMMAND = -2,
  ARCHIUM_ERROR_INVALID_ARGUMENT = -3,
  ARCHIUM_ERROR_BUFFER_OVERFLOW = -4,
  ARCHIUM_ERROR_SYSTEM_CALL = -10,
  ARCHIUM_ERROR_FILE_NOT_FOUND = -11,
  ARCHIUM_ERROR_FILE_ACCESS = -12,
  ARCHIUM_ERROR_MEMORY_ALLOCATION = -13,
  ARCHIUM_ERROR_PROCESS_FAILED = -14,
  ARCHIUM_ERROR_PACKAGE_MANAGER = -20,
  ARCHIUM_ERROR_PACKAGE_NOT_FOUND = -21,
  ARCHIUM_ERROR_PACKAGE_INSTALL_FAILED = -22,
  ARCHIUM_ERROR_PACKAGE_REMOVE_FAILED = -23,
  ARCHIUM_ERROR_PACKAGE_UPDATE_FAILED = -24,
  ARCHIUM_ERROR_PACKAGE_DEPENDENCY = -25,
  ARCHIUM_ERROR_NETWORK = -30,
  ARCHIUM_ERROR_DOWNLOAD_FAILED = -31,
  ARCHIUM_ERROR_CONNECTION_TIMEOUT = -32,
  ARCHIUM_ERROR_PERMISSION = -40,
  ARCHIUM_ERROR_PRIVILEGE_REQUIRED = -41,
  ARCHIUM_ERROR_ACCESS_DENIED = -42,
  ARCHIUM_ERROR_CONFIG = -50,
  ARCHIUM_ERROR_CONFIG_INVALID = -51,
  ARCHIUM_ERROR_CONFIG_MISSING = -52,
  ARCHIUM_ERROR_PLUGIN = -60,
  ARCHIUM_ERROR_PLUGIN_LOAD_FAILED = -61,
  ARCHIUM_ERROR_PLUGIN_INVALID = -62,
  ARCHIUM_ERROR_TIMEOUT = -70,
  ARCHIUM_ERROR_UNKNOWN = -99
} ArchiumError;

typedef struct {
  ArchiumError error_code;
  char context[ERROR_CONTEXT_MAX_LEN];
  char details[ERROR_DETAILS_MAX_LEN];
  char function[64];
  char file[128];
  int line;
  time_t timestamp;
  int system_errno;
  int retry_count;
} ArchiumErrorContext;

typedef ArchiumError (*ArchiumRetryFunc)(void *user_data);

const char *get_error_string(ArchiumError error_code);
void handle_error(ArchiumError error_code, const char *context);
void log_error(const char *error_message, ArchiumError error_code);
void archium_report_error(ArchiumError error_code, const char *context,
                          const char *input);
void archium_report_error_detailed(const ArchiumErrorContext *error_ctx);
ArchiumErrorContext *archium_create_error_context(ArchiumError code,
                                                  const char *context,
                                                  const char *function,
                                                  const char *file, int line);
void archium_error_add_details(ArchiumErrorContext *ctx, const char *details,
                               ...);
const char *archium_error_get_category(ArchiumError error_code);
int archium_error_is_recoverable(ArchiumError error_code);
ArchiumError archium_retry_operation(ArchiumRetryFunc func, void *user_data,
                                     int max_retries, int delay_ms,
                                     const char *operation_name);
void archium_suggest_recovery_action(ArchiumError error_code);

#endif