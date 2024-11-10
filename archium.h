#ifndef ARCHIUM_H
#define ARCHIUM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define LOG_FILE_PATH "/var/log/archium.log"

void log_action(const char *action);
void handle_signal(int signal);
int check_archium_file(void);
int check_package_manager(void);
int check_git(void);
void install_git(void);
void install_yay(void);
void update_system(const char *package_manager, const char *package);
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
void display_help(void);
void prompt_install_yay(void);
void get_input(char *input, const char *prompt);
int is_valid_command(const char *command);
void handle_command(const char *input, const char *package_manager);
void handle_exec_command(const char *command, const char *package_manager);
void display_version(void);
char **get_pacman_commands(void);
char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);
char *get_package_manager_version(const char *package_manager);
void cache_pacman_commands(void);

extern char **cached_commands;

#endif
