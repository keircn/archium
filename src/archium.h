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

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define MAX_RETRIES 3
#define TIMEOUT_SECONDS 30
#define ARCHIUM_VERSION "1.5.3"

typedef struct {
  int verbose;
  int version;
  int exec_mode;
  char *exec_command;
} ArchiumConfig;

typedef enum {
  ARCHIUM_SUCCESS = 0,
  ARCHIUM_ERROR_INVALID_INPUT = -1,
  ARCHIUM_ERROR_SYSTEM_CALL = -2,
  ARCHIUM_ERROR_PACKAGE_MANAGER = -3,
  ARCHIUM_ERROR_PERMISSION = -4,
  ARCHIUM_ERROR_TIMEOUT = -5
} ArchiumError;

const char *get_error_string(ArchiumError error_code);
void handle_error(ArchiumError error_code, const char *context);

extern ArchiumConfig config;
ArchiumError parse_arguments(int argc, char *argv[]);

int check_package_manager(void);
void prompt_install_yay(void);

void display_version(void);
void display_cli_help(void);
void display_help(void);

char **command_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);
void cache_pacman_commands(void);

void log_action(const char *action);
void log_error(const char *error_message, ArchiumError error_code);
void log_debug(const char *debug_message);
void log_info(const char *info_message);

void handle_signal(int signal);
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
char **get_pacman_commands(void);
char *command_generator(const char *text, int state);
char *get_package_manager_version(const char *package_manager);

extern char **cached_commands;

#endif
