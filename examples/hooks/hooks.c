#include <stdio.h>

#include <stddef.h>
#include <string.h>

typedef enum {
  ARCHIUM_SUCCESS = 0,
  ARCHIUM_ERROR_INVALID_INPUT = -1,
  ARCHIUM_ERROR_SYSTEM_CALL = -2
} ArchiumError;

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

int archium_plugin_get_api_version(void) { return ARCHIUM_PLUGIN_API_VERSION; }

char *archium_plugin_get_name(void) { return "Hooks Example"; }

char *archium_plugin_get_command(void) { return "hooks"; }

char *archium_plugin_get_description(void) {
  return "A simple plugin demonstrating hook callbacks";
}

void archium_plugin_init(const ArchiumPluginContext *ctx) {
  if (ctx && ctx->log_info) {
    ctx->log_info("Hooks example initialized");
  }
}

ArchiumError archium_plugin_before_command(const ArchiumPluginContext *ctx) {
  if (!ctx || !ctx->command) {
    return ARCHIUM_SUCCESS;
  }

  if (ctx->log_debug) {
    ctx->log_debug("Before command hook");
  }

  if (strcmp(ctx->command, "u") == 0 && ctx->log_info) {
    ctx->log_info("About to upgrade packages");
  }

  return ARCHIUM_SUCCESS;
}

void archium_plugin_after_command(const ArchiumPluginContext *ctx,
                                  ArchiumError result) {
  if (!ctx || !ctx->log_action) {
    return;
  }

  if (result == ARCHIUM_SUCCESS) {
    ctx->log_action("Command finished successfully");
  } else if (ctx->log_error) {
    ctx->log_error("Command failed", result);
  }
}

void archium_plugin_on_exit(const ArchiumPluginContext *ctx) {
  if (ctx && ctx->log_info) {
    ctx->log_info("Archium is exiting");
  }
}

ArchiumError archium_plugin_execute(const char *args,
                                    const char *package_manager) {
  (void)package_manager;
  printf("Hooks example executed\n");
  if (args && args[0] != '\0') {
    printf("Args: %s\n", args);
  }
  return ARCHIUM_SUCCESS;
}

void archium_plugin_cleanup(void) { /* no-op */ }
