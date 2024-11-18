#include "archium.h"
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <pwd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static const char* error_messages[] = {
    "Success",
    "Invalid input",
    "System call failed",
    "Package manager error",
    "Permission denied",
    "Operation timed out"
};

static char* get_log_path(const char* relative_path) {
    static char full_path[PATH_MAX];
    const char* home = getenv("HOME");
    
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            home = pwd->pw_dir;
        }
    }
    
    if (!home) {
        return NULL;
    }
    
    snprintf(full_path, sizeof(full_path), "%s/%s", home, relative_path);
    return full_path;
}

static void ensure_log_directory() {
    char log_dir[PATH_MAX];
    const char* home = getenv("HOME");
    
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            home = pwd->pw_dir;
        } else {
            return;
        }
    }
    
    snprintf(log_dir, sizeof(log_dir), "%s/.local", home);
    mkdir(log_dir, 0755);
    
    snprintf(log_dir, sizeof(log_dir), "%s/.local/share", home);
    mkdir(log_dir, 0755);
    
    snprintf(log_dir, sizeof(log_dir), "%s/.local/share/archium", home);
    mkdir(log_dir, 0755);
}

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

static void write_log(const char *relative_path, const char *level, const char *message) {
    ensure_log_directory();
    char* full_path = get_log_path(relative_path);
    if (!full_path) {
        return;
    }

    time_t now;
    char timestamp[26];
    FILE *fp;

    time(&now);
    ctime_r(&now, timestamp);
    timestamp[24] = '\0';

    fp = fopen(full_path, "a");
    if (fp) {
        fprintf(fp, "[%s] [%s] %s\n", timestamp, level, message);
        fclose(fp);
    }
}

void log_error(const char *error_message, ArchiumError error_code) {
    char full_message[COMMAND_BUFFER_SIZE];
    snprintf(full_message, sizeof(full_message), "%s (Error code: %d - %s)", 
             error_message, error_code, get_error_string(error_code));
    write_log(".local/share/archium/archium.log", "ERROR", full_message);
}

void log_debug(const char *debug_message) {
    write_log(".local/share/archium/archium.log", "DEBUG", debug_message);
}

void log_info(const char *info_message) {
    write_log(".local/share/archium/archium.log", "INFO", info_message);
}

void log_action(const char *action) {
    write_log(".local/share/archium/archium.log", "ACTION", action);
}
