#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#include "error.h"

typedef struct {
  int verbose;
  int version;
  int exec_mode;
  char *exec_command;
  int json_output;
  int batch_mode;
  int use_native_output;
  int show_welcome;
  int show_tips;
  int cache_ttl_seconds;
} ArchiumConfig;

extern ArchiumConfig config;

ArchiumError parse_arguments(int argc, char *argv[]);
int archium_config_init(void);
void archium_config_migrate_legacy_files(void);
const char *archium_config_get_config_dir(void);
const char *archium_config_get_log_file(void);
const char *archium_config_get_cache_dir(void);
const char *archium_config_get_plugin_dir(void);
const char *archium_config_get_preferred_package_manager(void);
int archium_config_check_paru_preference(void);
int archium_config_set_preference(const char *key, const char *value);
char *archium_config_get_preference(const char *key);
int archium_config_export_preferences(const char *file_path);
int archium_config_import_preferences(const char *file_path);
int archium_config_backup_preferences(char *out_path, size_t out_size);
int archium_config_restore_preferences(const char *file_path);
void archium_config_print_effective(FILE *out);
void archium_config_write_log(const char *level, const char *message);

#endif
