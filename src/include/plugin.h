#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>

#include "error.h"

#define ARCHIUM_PLUGIN_API_VERSION 2

typedef void (*ArchiumPluginLogFn)(const char *message);
typedef void (*ArchiumPluginLogErrorFn)(const char *message, ArchiumError code);
typedef int (*ArchiumPluginRunCommandFn)(const char *command,
                                         char *output_buffer,
                                         size_t output_size);

typedef struct {
  const char *package_manager;
  const char *command;
  const char *args;
  int verbose;
  const char *config_dir;
  const char *plugin_dir;
  const char *cache_dir;
  ArchiumPluginLogFn log_info;
  ArchiumPluginLogFn log_debug;
  ArchiumPluginLogFn log_action;
  ArchiumPluginLogErrorFn log_error;
  ArchiumPluginRunCommandFn run_command;
} ArchiumPluginContext;

int archium_plugin_init(void);
void archium_plugin_cleanup(void);
int archium_plugin_find_by_command(const char *command);
ArchiumError archium_plugin_execute(const char *command, const char *args,
                                    const char *package_manager);
int archium_plugin_is_plugin_command(const char *command);
ArchiumError archium_plugin_before_command(const char *command,
                                           const char *args,
                                           const char *package_manager);
void archium_plugin_after_command(const char *command, const char *args,
                                  const char *package_manager,
                                  ArchiumError result);
void archium_plugin_notify_exit(const char *command, const char *args,
                                const char *package_manager);
void archium_plugin_list_loaded(void);
void archium_plugin_display_help(void);
int archium_plugin_create_example(void);

#endif
