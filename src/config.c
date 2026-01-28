#include "include/archium.h"

#define CONFIG_BUFFER_SIZE 1024

static char config_dir_path[CONFIG_BUFFER_SIZE];
static char log_file_path[CONFIG_BUFFER_SIZE];
static char preference_file_path[CONFIG_BUFFER_SIZE];
static char cache_dir_path[CONFIG_BUFFER_SIZE];
static char plugin_dir_path[CONFIG_BUFFER_SIZE];

static int config_initialized = 0;

static int ensure_directory_exists(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) == -1) {
    if (mkdir(path, 0755) != 0) {
      return 0;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    return 0;
  }
  return 1;
}

static int init_config_paths(void) {
  const char *home = getenv("HOME");
  if (!home) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Unable to get HOME environment variable", NULL);
    return 0;
  }

  if (snprintf(config_dir_path, sizeof(config_dir_path), "%s/.config/archium",
               home) >= (int)sizeof(config_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Config directory path too long", NULL);
    return 0;
  }

  if (snprintf(log_file_path, sizeof(log_file_path), "%s/archium.log",
               config_dir_path) >= (int)sizeof(log_file_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL, "Log file path too long",
                         NULL);
    return 0;
  }

  if (snprintf(preference_file_path, sizeof(preference_file_path),
               "%s/preferences",
               config_dir_path) >= (int)sizeof(preference_file_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Preference file path too long", NULL);
    return 0;
  }

  if (snprintf(cache_dir_path, sizeof(cache_dir_path), "%s/cache",
               config_dir_path) >= (int)sizeof(cache_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Cache directory path too long", NULL);
    return 0;
  }

  if (snprintf(plugin_dir_path, sizeof(plugin_dir_path), "%s/plugins",
               config_dir_path) >= (int)sizeof(plugin_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Plugin directory path too long", NULL);
    return 0;
  }

  return 1;
}

int archium_config_init(void) {
  if (config_initialized) {
    return 1;
  }

  if (!init_config_paths()) {
    return 0;
  }

  char parent_config_dir[CONFIG_BUFFER_SIZE];
  const char *home = getenv("HOME");
  if (snprintf(parent_config_dir, sizeof(parent_config_dir), "%s/.config",
               home) >= (int)sizeof(parent_config_dir)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Parent config directory path too long", NULL);
    return 0;
  }

  if (!ensure_directory_exists(parent_config_dir)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create .config directory", NULL);
    return 0;
  }

  if (!ensure_directory_exists(config_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create Archium config directory", NULL);
    return 0;
  }

  if (!ensure_directory_exists(cache_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create Archium cache directory", NULL);
    return 0;
  }

  if (!ensure_directory_exists(plugin_dir_path)) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create Archium plugin directory", NULL);
    return 0;
  }

  config_initialized = 1;
  char *json_pref = archium_config_get_preference("json_output");
  if (json_pref) {
    if (strcmp(json_pref, "1") == 0 || strcmp(json_pref, "true") == 0) {
      config.json_output = 1;
    }
    free(json_pref);
  }

  char *batch_pref = archium_config_get_preference("batch_mode");
  if (batch_pref) {
    if (strcmp(batch_pref, "1") == 0 || strcmp(batch_pref, "true") == 0) {
      config.batch_mode = 1;
    }
    free(batch_pref);
  }
  return 1;
}

void archium_config_migrate_legacy_files(void) {
  if (!archium_config_init()) {
    return;
  }

  const char *home = getenv("HOME");
  if (!home) {
    return;
  }

  char legacy_paru_file[CONFIG_BUFFER_SIZE];
  if (snprintf(legacy_paru_file, sizeof(legacy_paru_file),
               "%s/.archium-use-paru", home) >= (int)sizeof(legacy_paru_file)) {
    return;
  }

  struct stat buffer;
  if (stat(legacy_paru_file, &buffer) == 0) {
    archium_config_set_preference("package_manager", "paru");
    if (unlink(legacy_paru_file) == 0) {
      log_info("Migrated legacy .archium-use-paru file to new config system");
    }
  }
}

const char *archium_config_get_config_dir(void) {
  if (!archium_config_init()) {
    return NULL;
  }
  return config_dir_path;
}

const char *archium_config_get_log_file(void) {
  if (!archium_config_init()) {
    return NULL;
  }
  return log_file_path;
}

const char *archium_config_get_cache_dir(void) {
  if (!archium_config_init()) {
    return NULL;
  }
  return cache_dir_path;
}

const char *archium_config_get_plugin_dir(void) {
  if (!archium_config_init()) {
    return NULL;
  }
  return plugin_dir_path;
}

int archium_config_check_paru_preference(void) {
  if (!archium_config_init()) {
    return 0;
  }

  char *pref = archium_config_get_preference("package_manager");
  if (pref && strcmp(pref, "paru") == 0) {
    free(pref);
    return 1;
  }
  if (pref) {
    free(pref);
  }
  return 0;
}

int archium_config_set_preference(const char *key, const char *value) {
  if (!archium_config_init()) {
    return 0;
  }

  FILE *fp = fopen(preference_file_path, "a+");
  if (!fp) {
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to open preferences file", NULL);
    return 0;
  }

  char line[CONFIG_BUFFER_SIZE];
  char temp_file[CONFIG_BUFFER_SIZE];
  int ret =
      snprintf(temp_file, sizeof(temp_file), "%s.tmp", preference_file_path);
  if (ret >= (int)sizeof(temp_file)) {
    fclose(fp);
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Temporary file path too long", NULL);
    return 0;
  }

  FILE *temp_fp = fopen(temp_file, "w");
  if (!temp_fp) {
    fclose(fp);
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to create temporary preferences file", NULL);
    return 0;
  }

  int found = 0;
  rewind(fp);
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';
    if (strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == '=') {
      fprintf(temp_fp, "%s=%s\n", key, value);
      found = 1;
    } else {
      fprintf(temp_fp, "%s\n", line);
    }
  }

  if (!found) {
    fprintf(temp_fp, "%s=%s\n", key, value);
  }

  if (strcmp(key, "json_output") == 0) {
    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0) {
      config.json_output = 1;
    } else {
      config.json_output = 0;
    }
  } else if (strcmp(key, "batch_mode") == 0) {
    if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0) {
      config.batch_mode = 1;
    } else {
      config.batch_mode = 0;
    }
  }

  fclose(fp);
  fclose(temp_fp);

  if (rename(temp_file, preference_file_path) != 0) {
    unlink(temp_file);
    archium_report_error(ARCHIUM_ERROR_SYSTEM_CALL,
                         "Failed to update preferences file", NULL);
    return 0;
  }

  return 1;
}

char *archium_config_get_preference(const char *key) {
  if (!archium_config_init()) {
    return NULL;
  }

  FILE *fp = fopen(preference_file_path, "r");
  if (!fp) {
    return NULL;
  }

  char line[CONFIG_BUFFER_SIZE];
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';
    if (strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == '=') {
      fclose(fp);
      return strdup(line + strlen(key) + 1);
    }
  }

  fclose(fp);
  return NULL;
}

void archium_config_write_log(const char *level, const char *message) {
  if (!config.verbose) {
    return;
  }

  if (!archium_config_init()) {
    fprintf(stderr, "[%s] %s\n", level, message);
    return;
  }

  FILE *fp = fopen(log_file_path, "a");
  if (!fp) {
    fprintf(stderr, "[%s] %s\n", level, message);
    return;
  }

  time_t now;
  char timestamp[26];

  time(&now);
  ctime_r(&now, timestamp);
  timestamp[24] = '\0';

  fprintf(fp, "[%s] [%s] %s\n", timestamp, level, message);
  fclose(fp);
}
