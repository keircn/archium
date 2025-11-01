#ifndef PLUGIN_H
#define PLUGIN_H

#include "error.h"

int archium_plugin_init(void);
void archium_plugin_cleanup(void);
int archium_plugin_find_by_command(const char *command);
ArchiumError archium_plugin_execute(const char *command, const char *args,
                                    const char *package_manager);
int archium_plugin_is_plugin_command(const char *command);
void archium_plugin_list_loaded(void);
void archium_plugin_display_help(void);
int archium_plugin_create_example(void);

#endif