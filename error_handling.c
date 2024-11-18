#include "archium.h"
#include <errno.h>
#include <stdarg.h>
#include <time.h>

static const char* error_messages[] = {
    [ARCHIUM_SUCCESS] = "Success",
    [ARCHIUM_ERROR_INVALID_INPUT] = "Invalid input",
    [ARCHIUM_ERROR_SYSTEM_CALL] = "System call failed",
    [ARCHIUM_ERROR_PACKAGE_MANAGER] = "Package manager error",
    [ARCHIUM_ERROR_PERMISSION] = "Permission denied",
    [ARCHIUM_ERROR_TIMEOUT] = "Operation timed out"
};

const char* get_error_string(ArchiumError error_code) {
    if (error_code >= 0 || -error_code >= sizeof(error_messages)/sizeof(error_messages[0])) {
        return "Unknown error";
    }
    return error_messages[-error_code];
}

void handle_error(ArchiumError error_code, const char *context) {
    const char *error_str = get_error_string(error_code);
    fprintf(stderr, "\033[1;31mError: %s - %s\033[0m\n", context, error_str);
}

static void write_log(const char *log_file, const char *level, const char *format, va_list args) {
    time_t now;
    char timestamp[26];
    FILE *fp;

    time(&now);
    ctime_r(&now, timestamp);
    timestamp[24] = '\0';

    fp = fopen(log_file, "a");
    if (fp) {
        fprintf(fp, "[%s] [%s] ", timestamp, level);
        vfprintf(fp, format, args);
        fprintf(fp, "\n");
        fclose(fp);
    }
}

void log_error(const char *error_message, ArchiumError error_code) {
    va_list args;
    va_start(args, error_message);
    
    char full_message[COMMAND_BUFFER_SIZE];
    snprintf(full_message, sizeof(full_message), "%s (Error code: %d - %s)", 
             error_message, error_code, get_error_string(error_code));
    
    write_log(ERROR_LOG_FILE_PATH, "ERROR", full_message, args);
    va_end(args);
}

void log_debug(const char *debug_message) {
    va_list args;
    va_start(args, debug_message);
    write_log(LOG_FILE_PATH, "DEBUG", debug_message, args);
    va_end(args);
}

void log_info(const char *info_message) {
    va_list args;
    va_start(args, info_message);
    write_log(LOG_FILE_PATH, "INFO", info_message, args);
    va_end(args);
}

void log_action(const char *action) {
    va_list args;
    va_start(args, action);
    write_log(LOG_FILE_PATH, "ACTION", action, args);
    va_end(args);
}
