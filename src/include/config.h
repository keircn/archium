#ifndef CONFIG_H
#define CONFIG_H

#include "error.h"

typedef struct {
  int verbose;
  int version;
  int exec_mode;
  char *exec_command;
} ArchiumConfig;

extern ArchiumConfig config;

ArchiumError parse_arguments(int argc, char *argv[]);
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

#endif