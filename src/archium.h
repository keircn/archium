#ifndef ARCHIUM_H
#define ARCHIUM_H

#include <stdio.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "version.h"

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define MAX_RETRIES 3
#define TIMEOUT_SECONDS 30
#define ARCHIUM_REPO_URL "https://github.com/keircn/archium.git"
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

typedef struct {
  int verbose;
  int version;
  int exec_mode;
  char *exec_command;
} ArchiumConfig;

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

const char *get_error_string(ArchiumError error_code);
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

typedef ArchiumError (*ArchiumRetryFunc)(void *user_data);
ArchiumError archium_retry_operation(ArchiumRetryFunc func, void *user_data,
                                     int max_retries, int delay_ms,
                                     const char *operation_name);
void archium_suggest_recovery_action(ArchiumError error_code);

extern ArchiumConfig config;
ArchiumError parse_arguments(int argc, char *argv[]);

int check_package_manager(void);
void prompt_install_yay(void);

void display_version(void);
void display_cli_help(void);
void display_help(void);
void display_help_category(const char *category);
void display_help_quick(void);
void display_help_command(const char *command);
void display_random_tip(void);

char **command_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);
void cache_pacman_commands(void);
void cleanup_cached_commands(void);

void log_action(const char *action);
void log_debug(const char *debug_message);
void log_info(const char *info_message);

void handle_signal(int signal);
void show_progress_bar(int current, int total, const char *prefix);
void show_spinner(int position, const char *message);
int execute_command_with_spinner(const char *command, const char *message);
int execute_command_with_output_capture(const char *command,
                                        const char *message,
                                        char *output_buffer,
                                        size_t buffer_size);
void parse_and_show_upgrade_result(const char *output, int exit_code);
void parse_and_show_install_result(const char *output, int exit_code,
                                   const char *package);
void parse_and_show_remove_result(const char *output, int exit_code,
                                  const char *package);
void parse_and_show_generic_result(const char *output, int exit_code,
                                   const char *operation);
int check_archium_file(void);
void install_git(void);
void install_yay(void);
void update_system(const char *package_manager, const char *package);
ArchiumError handle_exec_command(const char *command,
                                 const char *package_manager);
ArchiumError handle_command(const char *input, const char *package_manager);
void install_package(const char *package_manager, const char *packages);
void remove_package(const char *package_manager, const char *packages);
void purge_package(const char *package_manager, const char *packages);
void clean_cache(const char *package_manager);
void clean_orphans(const char *package_manager);
void search_package(const char *package_manager, const char *package);
void list_installed_packages(void);
void show_package_info(const char *package_manager, const char *package);
void check_package_updates(void);
void display_dependency_tree(const char *package_manager, const char *package);
void clear_build_cache(void);
void list_orphans(void);
void get_input(char *input, const char *prompt);
int is_valid_command(const char *command);
void list_packages_by_size(void);
void list_recent_installs(void);
void list_explicit_installs(void);
void find_package_owner(const char *file);
void backup_pacman_config(void);
void configure_preferences(void);
char **get_pacman_commands(void);
char *command_generator(const char *text, int state);
char *get_package_manager_version(const char *package_manager);
void perform_self_update(void);

int archium_config_init(void);
void archium_config_migrate_legacy_files(void);
const char *archium_config_get_config_dir(void);
const char *archium_config_get_log_file(void);
const char *archium_config_get_cache_dir(void);
const char *archium_config_get_plugin_dir(void);
int archium_config_check_paru_preference(void);
int archium_config_set_preference(const char *key, const char *value);
char *archium_config_get_preference(const char *key);
void archium_config_write_log(const char *level, const char *message);

int archium_plugin_init(void);
void archium_plugin_cleanup(void);
int archium_plugin_find_by_command(const char *command);
ArchiumError archium_plugin_execute(const char *command, const char *args,
                                    const char *package_manager);
int archium_plugin_is_plugin_command(const char *command);
void archium_plugin_list_loaded(void);
void archium_plugin_display_help(void);
int archium_plugin_create_example(void);

extern char **cached_commands;

#endif
