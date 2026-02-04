#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include "include/archium.h"

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_PLUGINS 32
#define MAX_PLUGIN_NAME_LENGTH 64
#define MAX_PLUGIN_COMMAND_LENGTH 32

typedef struct {
  char name[MAX_PLUGIN_NAME_LENGTH];
  char command[MAX_PLUGIN_COMMAND_LENGTH];
  char description[256];
  void *handle;
  ArchiumError (*execute)(const char *args, const char *package_manager);
  int (*get_api_version)(void);
  void (*init)(const ArchiumPluginContext *ctx);
  ArchiumError (*before_command)(const ArchiumPluginContext *ctx);
  void (*after_command)(const ArchiumPluginContext *ctx, ArchiumError result);
  void (*on_exit)(const ArchiumPluginContext *ctx);
  void (*cleanup)(void);
} ArchiumPlugin;

static ArchiumPlugin loaded_plugins[MAX_PLUGINS];
static int plugin_count = 0;
static ArchiumPluginContext base_context = {0};

static int archium_plugin_run_command(const char *command, char *output_buffer,
                                      size_t output_size) {
  return execute_command_with_output_capture(command, NULL, output_buffer,
                                             output_size);
}

static void archium_plugin_set_base_context(void) {
  base_context.package_manager = NULL;
  base_context.command = NULL;
  base_context.args = NULL;
  base_context.verbose = config.verbose;
  base_context.config_dir = archium_config_get_config_dir();
  base_context.plugin_dir = archium_config_get_plugin_dir();
  base_context.cache_dir = archium_config_get_cache_dir();
  base_context.log_info = log_info;
  base_context.log_debug = log_debug;
  base_context.log_action = log_action;
  base_context.log_error = log_error;
  base_context.run_command = archium_plugin_run_command;
}

static void archium_plugin_fill_context(ArchiumPluginContext *ctx,
                                        const char *command, const char *args,
                                        const char *package_manager) {
  if (!ctx) {
    return;
  }
  archium_plugin_set_base_context();
  *ctx = base_context;
  ctx->command = command;
  ctx->args = args;
  ctx->package_manager = package_manager;
}

static int is_valid_plugin_file(const char *filename) {
  size_t len = strlen(filename);
  if (len < 3) return 0;
  return strcmp(filename + len - 3, ".so") == 0;
}

int archium_plugin_init(void) {
  const char *plugin_dir = archium_config_get_plugin_dir();
  if (!plugin_dir) {
    return 0;
  }

  archium_plugin_set_base_context();

  DIR *dir = opendir(plugin_dir);
  if (!dir) {
    return 1;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL && plugin_count < MAX_PLUGINS) {
    if (entry->d_type != DT_REG || !is_valid_plugin_file(entry->d_name)) {
      continue;
    }

    char plugin_path[1024];
    int ret = snprintf(plugin_path, sizeof(plugin_path), "%s/%s", plugin_dir,
                       entry->d_name);
    if (ret >= (int)sizeof(plugin_path)) {
      continue;
    }

    void *handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
      log_debug("Failed to load plugin");
      continue;
    }

    char *(*get_name)(void) = dlsym(handle, "archium_plugin_get_name");
    char *(*get_command)(void) = dlsym(handle, "archium_plugin_get_command");
    char *(*get_description)(void) =
        dlsym(handle, "archium_plugin_get_description");
    ArchiumError (*execute)(const char *, const char *) =
        dlsym(handle, "archium_plugin_execute");
    int (*get_api_version)(void) =
        dlsym(handle, "archium_plugin_get_api_version");
    void (*init)(const ArchiumPluginContext *) =
        dlsym(handle, "archium_plugin_init");
    ArchiumError (*before_command)(const ArchiumPluginContext *) =
        dlsym(handle, "archium_plugin_before_command");
    void (*after_command)(const ArchiumPluginContext *, ArchiumError) =
        dlsym(handle, "archium_plugin_after_command");
    void (*on_exit)(const ArchiumPluginContext *) =
        dlsym(handle, "archium_plugin_on_exit");
    void (*cleanup)(void) = dlsym(handle, "archium_plugin_cleanup");

    if (!get_name || !get_command || !get_description || !execute) {
      dlclose(handle);
      continue;
    }

    char *name = get_name();
    char *command = get_command();
    char *description = get_description();

    if (!name || !command || !description) {
      dlclose(handle);
      continue;
    }

    if (strlen(name) >= MAX_PLUGIN_NAME_LENGTH ||
        strlen(command) >= MAX_PLUGIN_COMMAND_LENGTH ||
        strlen(description) >= 256) {
      dlclose(handle);
      continue;
    }

    if (archium_plugin_find_by_command(command) != -1) {
      log_debug("Plugin command already exists, skipping");
      dlclose(handle);
      continue;
    }

    strcpy(loaded_plugins[plugin_count].name, name);
    strcpy(loaded_plugins[plugin_count].command, command);
    strcpy(loaded_plugins[plugin_count].description, description);
    loaded_plugins[plugin_count].handle = handle;
    loaded_plugins[plugin_count].execute = execute;
    loaded_plugins[plugin_count].get_api_version = get_api_version;
    loaded_plugins[plugin_count].init = init;
    loaded_plugins[plugin_count].before_command = before_command;
    loaded_plugins[plugin_count].after_command = after_command;
    loaded_plugins[plugin_count].on_exit = on_exit;
    loaded_plugins[plugin_count].cleanup = cleanup;

    plugin_count++;
    log_info("Loaded plugin");

    ArchiumPluginContext ctx;
    archium_plugin_fill_context(&ctx, NULL, NULL, NULL);
    if (loaded_plugins[plugin_count - 1].get_api_version &&
        loaded_plugins[plugin_count - 1].get_api_version() <
            ARCHIUM_PLUGIN_API_VERSION) {
      log_debug("Plugin uses legacy API version");
    }
    if (loaded_plugins[plugin_count - 1].init) {
      loaded_plugins[plugin_count - 1].init(&ctx);
    }
  }

  closedir(dir);
  return 1;
}

void archium_plugin_cleanup(void) {
  for (int i = 0; i < plugin_count; i++) {
    if (loaded_plugins[i].cleanup) {
      loaded_plugins[i].cleanup();
    }
    if (loaded_plugins[i].handle) {
      dlclose(loaded_plugins[i].handle);
    }
  }
  plugin_count = 0;
}

int archium_plugin_find_by_command(const char *command) {
  if (!command) {
    return -1;
  }
  for (int i = 0; i < plugin_count; i++) {
    if (strcmp(loaded_plugins[i].command, command) == 0) {
      return i;
    }
  }
  return -1;
}

ArchiumError archium_plugin_execute(const char *command, const char *args,
                                    const char *package_manager) {
  int index = archium_plugin_find_by_command(command);
  if (index == -1) {
    return ARCHIUM_ERROR_INVALID_INPUT;
  }

  return loaded_plugins[index].execute(args ? args : "", package_manager);
}

ArchiumError archium_plugin_before_command(const char *command,
                                           const char *args,
                                           const char *package_manager) {
  ArchiumPluginContext ctx;
  archium_plugin_fill_context(&ctx, command, args, package_manager);

  for (int i = 0; i < plugin_count; i++) {
    if (!loaded_plugins[i].before_command) {
      continue;
    }

    ArchiumError result = loaded_plugins[i].before_command(&ctx);
    if (result != ARCHIUM_SUCCESS) {
      return result;
    }
  }

  return ARCHIUM_SUCCESS;
}

void archium_plugin_after_command(const char *command, const char *args,
                                  const char *package_manager,
                                  ArchiumError result) {
  ArchiumPluginContext ctx;
  archium_plugin_fill_context(&ctx, command, args, package_manager);

  for (int i = 0; i < plugin_count; i++) {
    if (!loaded_plugins[i].after_command) {
      continue;
    }
    loaded_plugins[i].after_command(&ctx, result);
  }
}

void archium_plugin_notify_exit(const char *command, const char *args,
                                const char *package_manager) {
  ArchiumPluginContext ctx;
  archium_plugin_fill_context(&ctx, command, args, package_manager);

  for (int i = 0; i < plugin_count; i++) {
    if (!loaded_plugins[i].on_exit) {
      continue;
    }
    loaded_plugins[i].on_exit(&ctx);
  }
}

int archium_plugin_is_plugin_command(const char *command) {
  if (!command) {
    return 0;
  }

  while (*command == ' ') {
    command++;
  }

  char token[MAX_PLUGIN_COMMAND_LENGTH];
  const char *space = strchr(command, ' ');
  size_t token_len = space ? (size_t)(space - command) : strlen(command);
  if (token_len >= sizeof(token)) {
    token_len = sizeof(token) - 1;
  }
  memcpy(token, command, token_len);
  token[token_len] = '\0';

  return archium_plugin_find_by_command(token) != -1;
}

void archium_plugin_list_loaded(void) {
  archium_plugin_set_base_context();
  if (plugin_count == 0) {
    printf("\033[1;33mNo plugins loaded.\033[0m\n");
    return;
  }

  printf("\033[1;34mLoaded plugins:\033[0m\n");
  for (int i = 0; i < plugin_count; i++) {
    printf("\033[1;32m%s\033[0m (\033[1;36m%s\033[0m) - %s\n",
           loaded_plugins[i].command, loaded_plugins[i].name,
           loaded_plugins[i].description);
  }
}

void archium_plugin_display_help(void) {
  archium_plugin_set_base_context();
  if (plugin_count == 0) {
    return;
  }

  printf("\n\033[1;33mPlugin commands:\033[0m\n");
  for (int i = 0; i < plugin_count; i++) {
    printf("\033[1;32m%s\033[0m%*s - %s\n", loaded_plugins[i].command,
           (int)(12 - strlen(loaded_plugins[i].command)), "",
           loaded_plugins[i].description);
  }
}

int archium_plugin_create_example(void) {
  const char *plugin_dir = archium_config_get_plugin_dir();
  if (!plugin_dir) {
    return 0;
  }

  archium_plugin_set_base_context();

  char example_path[1024];
  int ret =
      snprintf(example_path, sizeof(example_path), "%s/example.c", plugin_dir);
  if (ret >= (int)sizeof(example_path)) {
    return 0;
  }

  FILE *fp = fopen(example_path, "w");
  if (!fp) {
    return 0;
  }

  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include <stddef.h>\n\n");
  fprintf(fp, "typedef enum {\n");
  fprintf(fp, "  ARCHIUM_SUCCESS = 0,\n");
  fprintf(fp, "  ARCHIUM_ERROR_INVALID_INPUT = -1,\n");
  fprintf(fp, "  ARCHIUM_ERROR_SYSTEM_CALL = -2\n");
  fprintf(fp, "} ArchiumError;\n\n");
  fprintf(fp, "#define ARCHIUM_PLUGIN_API_VERSION 2\n\n");
  fprintf(fp, "typedef void (*ArchiumPluginLogFn)(const char *message);\n");
  fprintf(fp,
          "typedef void (*ArchiumPluginLogErrorFn)(const char *message, "
          "ArchiumError code);\n");
  fprintf(fp,
          "typedef int (*ArchiumPluginRunCommandFn)(const char *command, "
          "char *output_buffer, size_t output_size);\n\n");
  fprintf(fp, "typedef struct {\n");
  fprintf(fp, "  const char *package_manager;\n");
  fprintf(fp, "  const char *command;\n");
  fprintf(fp, "  const char *args;\n");
  fprintf(fp, "  int verbose;\n");
  fprintf(fp, "  const char *config_dir;\n");
  fprintf(fp, "  const char *plugin_dir;\n");
  fprintf(fp, "  const char *cache_dir;\n");
  fprintf(fp, "  ArchiumPluginLogFn log_info;\n");
  fprintf(fp, "  ArchiumPluginLogFn log_debug;\n");
  fprintf(fp, "  ArchiumPluginLogFn log_action;\n");
  fprintf(fp, "  ArchiumPluginLogErrorFn log_error;\n");
  fprintf(fp, "  ArchiumPluginRunCommandFn run_command;\n");
  fprintf(fp, "} ArchiumPluginContext;\n\n");
  fprintf(fp, "int archium_plugin_get_api_version(void) {\n");
  fprintf(fp, "  return ARCHIUM_PLUGIN_API_VERSION;\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "char *archium_plugin_get_name(void) {\n");
  fprintf(fp, "  return \"Example Plugin\";\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "char *archium_plugin_get_command(void) {\n");
  fprintf(fp, "  return \"example\";\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "char *archium_plugin_get_description(void) {\n");
  fprintf(fp,
          "  return \"An example plugin that demonstrates the plugin API\";\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "void archium_plugin_init(const ArchiumPluginContext *ctx) {\n");
  fprintf(fp, "  if (ctx && ctx->log_info) {\n");
  fprintf(fp, "    ctx->log_info(\"Example plugin initialized\");\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "}\n\n");
  fprintf(fp,
          "ArchiumError archium_plugin_before_command(const "
          "ArchiumPluginContext *ctx) {\n");
  fprintf(fp,
          "  if (ctx && ctx->command && strcmp(ctx->command, \"u\") == 0) "
          "{\n");
  fprintf(fp, "    if (ctx->log_info) {\n");
  fprintf(fp, "      ctx->log_info(\"Preparing system upgrade\");\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  return ARCHIUM_SUCCESS;\n");
  fprintf(fp, "}\n\n");
  fprintf(fp,
          "void archium_plugin_after_command(const ArchiumPluginContext *ctx, "
          "ArchiumError result) {\n");
  fprintf(fp, "  if (ctx && ctx->log_debug) {\n");
  fprintf(fp, "    ctx->log_debug(\"Command completed\");\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  (void)result;\n");
  fprintf(fp, "}\n\n");
  fprintf(fp,
          "void archium_plugin_on_exit(const ArchiumPluginContext *ctx) "
          "{\n");
  fprintf(fp, "  if (ctx && ctx->log_info) {\n");
  fprintf(fp, "    ctx->log_info(\"Archium exiting\");\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "}\n\n");
  fprintf(fp,
          "ArchiumError archium_plugin_execute(const char *args, const char "
          "*package_manager) {\n");
  fprintf(fp,
          "  printf(\"\\033[1;32mExample plugin executed!\\033[0m\\n\");\n");
  fprintf(fp, "  if (args && strlen(args) > 0) {\n");
  fprintf(fp, "    printf(\"Arguments: %%s\\n\", args);\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  printf(\"Package manager: %%s\\n\", package_manager);\n");
  fprintf(fp, "  return ARCHIUM_SUCCESS;\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "void archium_plugin_cleanup(void) {\n");
  fprintf(fp, "  // Cleanup code here\n");
  fprintf(fp, "}\n");

  fclose(fp);

  char makefile_path[1024];
  ret =
      snprintf(makefile_path, sizeof(makefile_path), "%s/Makefile", plugin_dir);
  if (ret >= (int)sizeof(makefile_path)) {
    return 1;
  }

  fp = fopen(makefile_path, "w");
  if (!fp) {
    return 1;
  }

  fprintf(fp, "CC = gcc\n");
  fprintf(fp, "CFLAGS = -Wall -Wextra -fPIC\n");
  fprintf(fp, "LDFLAGS = -shared\n\n");
  fprintf(fp, "all: example.so\n\n");
  fprintf(fp, "example.so: example.c\n");
  fprintf(fp, "\t$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<\n\n");
  fprintf(fp, "clean:\n");
  fprintf(fp, "\trm -f *.so\n\n");
  fprintf(fp, ".PHONY: all clean\n");

  fclose(fp);
  return 1;
}
