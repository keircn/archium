#include <stdio.h>

typedef enum {
  ARCHIUM_SUCCESS = 0,
  ARCHIUM_ERROR_INVALID_INPUT = -1,
  ARCHIUM_ERROR_SYSTEM_CALL = -2
} ArchiumError;

char *archium_plugin_get_name(void) { return "Hello Plugin"; }

char *archium_plugin_get_command(void) { return "hello"; }

char *archium_plugin_get_description(void) {
  return "A hello world plugin";
}

ArchiumError archium_plugin_execute(const char *args,
                                    const char *package_manager) {
  (void)package_manager;
  printf("Hello from Archium!\n");
  if (args && args[0] != '\0') {
    printf("Args: %s\n", args);
  }
  return ARCHIUM_SUCCESS;
}

void archium_plugin_cleanup(void) { /* no-op */ }
