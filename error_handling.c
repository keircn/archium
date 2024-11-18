#include "archium.h"
#include <time.h>

static const char* error_messages[] = {
    "Success",
    "Invalid input",
    "System call failed",
    "Package manager error",
    "Permission denied",
    "Operation timed out"
};

const char* get_error_string(ArchiumError error_code) {
    size_t index = (error_code < 0) ? (size_t)(-error_code) : (size_t)error_code;
    if (index >= sizeof(error_messages)/sizeof(error_messages[0])) {
        return "Unknown error";
    }
    return error_messages[index];
}

void handle_error(ArchiumError error_code, const char *context) {
    const char *error_str = get_error_string(error_code);
    fprintf(stderr, "\033[1;31mError: %s - %s\033[0m\n", context, error_str);
}

static void write_log(const char *level, const char *message) {
    time_t now;
    char timestamp[26];
    
    time(&now);
    ctime_r(&now, timestamp);
    timestamp[24] = '\0';  // Remove newline

    fprintf(stderr, "[%s] [%s] %s\n", timestamp, level, message);
}

void log_error(const char *error_message, ArchiumError error_code) {
    char full_message[COMMAND_BUFFER_SIZE];
    snprintf(full_message, sizeof(full_message), "%s (Error code: %d - %s)", 
             error_message, error_code, get_error_string(error_code));
    write_log("ERROR", full_message);
}

void log_debug(const char *debug_message) {
    write_log("DEBUG", debug_message);
}

void log_info(const char *info_message) {
    write_log("INFO", info_message);
}

void log_action(const char *action) {
    write_log("ACTION", action);
}
