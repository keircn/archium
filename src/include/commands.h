#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"
#include "error.h"

ArchiumError handle_exec_command(const char *command,
                                 const char *package_manager);
ArchiumError handle_command(const char *input, const char *package_manager);
void get_input(char *input, const char *prompt);
int is_valid_command(const char *command);
int check_archium_file(void);
void install_git(void);
void perform_self_update(void);
void downgrade_package(const char *package_manager, const char *packages);
char **list_cached_versions(const char *package, int *count);
char **list_cached_versions(const char *package, int *count);

#endif