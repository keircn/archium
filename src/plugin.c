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
  void (*cleanup)(void);
} ArchiumPlugin;

static ArchiumPlugin loaded_plugins[MAX_PLUGINS];
static int plugin_count = 0;

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
    loaded_plugins[plugin_count].cleanup = cleanup;

    plugin_count++;
    log_info("Loaded plugin");
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

  return loaded_plugins[index].execute(args, package_manager);
}

int archium_plugin_is_plugin_command(const char *command) {
  return archium_plugin_find_by_command(command) != -1;
}

void archium_plugin_list_loaded(void) {
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
  fprintf(fp, "#include <string.h>\n\n");
  fprintf(fp, "typedef enum {\n");
  fprintf(fp, "  ARCHIUM_SUCCESS = 0,\n");
  fprintf(fp, "  ARCHIUM_ERROR_INVALID_INPUT = -1,\n");
  fprintf(fp, "  ARCHIUM_ERROR_SYSTEM_CALL = -2\n");
  fprintf(fp, "} ArchiumError;\n\n");
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
