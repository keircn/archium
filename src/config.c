#include "include/archium.h"

#define CONFIG_BUFFER_SIZE 1024
#define CONFIG_KEY_MAX_LEN 64
#define CONFIG_VALUE_MAX_LEN 256
#define CONFIG_CACHE_TTL_MIN 60
#define CONFIG_CACHE_TTL_MAX 86400

static char config_dir_path[CONFIG_BUFFER_SIZE];
static char log_file_path[CONFIG_BUFFER_SIZE];
static char preference_file_path[CONFIG_BUFFER_SIZE];
static char cache_dir_path[CONFIG_BUFFER_SIZE];
static char plugin_dir_path[CONFIG_BUFFER_SIZE];
static char preferred_package_manager[16] = "";

static int config_initialized = 0;

static int parse_int_in_range(const char *value, int min, int max, int *out) {
  if (!value || !out) {
    return 0;
  }

  char *endptr = NULL;
  long parsed = strtol(value, &endptr, 10);
  if (endptr == value || *endptr != '\0') {
    return 0;
  }

  if (parsed < min || parsed > max) {
    return 0;
  }

  *out = (int)parsed;
  return 1;
}

static int parse_bool_value(const char *value, int *out) {
  if (!value || !out) {
    return 0;
  }

  if (strcmp(value, "1") == 0 || strcmp(value, "true") == 0) {
    *out = 1;
    return 1;
  }

  if (strcmp(value, "0") == 0 || strcmp(value, "false") == 0) {
    *out = 0;
    return 1;
  }

  return 0;
}

static int is_boolean_value(const char *value) {
  if (!value) {
    return 0;
  }

  return strcmp(value, "0") == 0 || strcmp(value, "1") == 0 ||
         strcmp(value, "false") == 0 || strcmp(value, "true") == 0;
}

static int validate_preference_key_value(const char *key, const char *value) {
  if (!key || !value || key[0] == '\0' || value[0] == '\0') {
    return 0;
  }

  if (strlen(key) >= CONFIG_KEY_MAX_LEN ||
      strlen(value) >= CONFIG_VALUE_MAX_LEN) {
    return 0;
  }

  if (strcmp(key, "package_manager") == 0) {
    return strcmp(value, "yay") == 0 || strcmp(value, "paru") == 0;
  }

  if (strcmp(key, "json_output") == 0 || strcmp(key, "batch_mode") == 0) {
    return is_boolean_value(value);
  }

  if (strcmp(key, "use_native_output") == 0 ||
      strcmp(key, "show_welcome") == 0 || strcmp(key, "show_tips") == 0) {
    return is_boolean_value(value);
  }

  if (strcmp(key, "cache_ttl_seconds") == 0) {
    int ttl = 0;
    return parse_int_in_range(value, CONFIG_CACHE_TTL_MIN, CONFIG_CACHE_TTL_MAX,
                              &ttl);
  }

  return 0;
}

static void apply_preference_to_runtime(const char *key, const char *value) {
  int bool_value = 0;
  int int_value = 0;

  if (strcmp(key, "json_output") == 0 && parse_bool_value(value, &bool_value)) {
    config.json_output = bool_value;
    return;
  }

  if (strcmp(key, "package_manager") == 0) {
    snprintf(preferred_package_manager, sizeof(preferred_package_manager), "%s",
             value);
    return;
  }

  if (strcmp(key, "batch_mode") == 0 && parse_bool_value(value, &bool_value)) {
    config.batch_mode = bool_value;
    return;
  }

  if (strcmp(key, "use_native_output") == 0 &&
      parse_bool_value(value, &bool_value)) {
    config.use_native_output = bool_value;
    return;
  }

  if (strcmp(key, "show_welcome") == 0 &&
      parse_bool_value(value, &bool_value)) {
    config.show_welcome = bool_value;
    return;
  }

  if (strcmp(key, "show_tips") == 0 && parse_bool_value(value, &bool_value)) {
    config.show_tips = bool_value;
    return;
  }

  if (strcmp(key, "cache_ttl_seconds") == 0 &&
      parse_int_in_range(value, CONFIG_CACHE_TTL_MIN, CONFIG_CACHE_TTL_MAX,
                         &int_value)) {
    config.cache_ttl_seconds = int_value;
  }
}

static void apply_preference_if_exists(const char *key) {
  char *value = archium_config_get_preference(key);
  if (!value) {
    return;
  }

  apply_preference_to_runtime(key, value);
  free(value);
}

static void apply_env_override(const char *env_name, const char *key) {
  const char *env_value = getenv(env_name);
  if (!env_value || env_value[0] == '\0') {
    return;
  }

  if (validate_preference_key_value(key, env_value)) {
    apply_preference_to_runtime(key, env_value);
  } else if (config.verbose) {
    char msg[CONFIG_BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "Ignoring invalid env override: %s", env_name);
    log_debug(msg);
  }
}

static void apply_environment_overrides(void) {
  apply_env_override("ARCHIUM_PACKAGE_MANAGER", "package_manager");
  apply_env_override("ARCHIUM_JSON_OUTPUT", "json_output");
  apply_env_override("ARCHIUM_BATCH_MODE", "batch_mode");
  apply_env_override("ARCHIUM_NATIVE_OUTPUT", "use_native_output");
  apply_env_override("ARCHIUM_SHOW_WELCOME", "show_welcome");
  apply_env_override("ARCHIUM_SHOW_TIPS", "show_tips");
  apply_env_override("ARCHIUM_CACHE_TTL_SECONDS", "cache_ttl_seconds");
}

static int validate_preference_line(const char *line) {
  if (!line) {
    return 0;
  }

  if (line[0] == '\0' || line[0] == '#') {
    return 1;
  }

  const char *sep = strchr(line, '=');
  if (!sep || sep == line || *(sep + 1) == '\0') {
    return 0;
  }

  char key[CONFIG_KEY_MAX_LEN];
  char value[CONFIG_VALUE_MAX_LEN];
  size_t key_len = (size_t)(sep - line);
  size_t value_len = strlen(sep + 1);

  if (key_len == 0 || key_len >= sizeof(key) || value_len == 0 ||
      value_len >= sizeof(value)) {
    return 0;
  }

  memcpy(key, line, key_len);
  key[key_len] = '\0';
  memcpy(value, sep + 1, value_len + 1);

  return validate_preference_key_value(key, value);
}

static int copy_file_contents(const char *src_path, const char *dst_path) {
  FILE *src = fopen(src_path, "r");
  if (!src) {
    return 0;
  }

  FILE *dst = fopen(dst_path, "w");
  if (!dst) {
    fclose(src);
    return 0;
  }

  char buffer[CONFIG_BUFFER_SIZE];
  size_t bytes_read = 0;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
    if (fwrite(buffer, 1, bytes_read, dst) != bytes_read) {
      fclose(src);
      fclose(dst);
      return 0;
    }
  }

  fclose(src);
  fclose(dst);
  return 1;
}

static int validate_preferences_file(void) {
  FILE *fp = fopen(preference_file_path, "r");
  if (!fp) {
    return 1;
  }

  char line[CONFIG_BUFFER_SIZE];
  int line_number = 0;
  int all_valid = 1;

  while (fgets(line, sizeof(line), fp)) {
    line_number++;
    line[strcspn(line, "\n")] = '\0';

    if (!validate_preference_line(line)) {
      all_valid = 0;
      if (config.verbose) {
        char msg[CONFIG_BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "Invalid preference on line %d",
                 line_number);
        log_debug(msg);
      }
    }
  }

  fclose(fp);
  return all_valid;
}

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

  if (!validate_preferences_file()) {
    archium_report_error(ARCHIUM_ERROR_CONFIG_INVALID,
                         "Preferences file contains invalid entries", NULL);
  }

  apply_preference_if_exists("json_output");
  apply_preference_if_exists("batch_mode");
  apply_preference_if_exists("package_manager");
  apply_preference_if_exists("use_native_output");
  apply_preference_if_exists("show_welcome");
  apply_preference_if_exists("show_tips");
  apply_preference_if_exists("cache_ttl_seconds");

  apply_environment_overrides();

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

  return strcmp(archium_config_get_preferred_package_manager(), "paru") == 0;
}

const char *archium_config_get_preferred_package_manager(void) {
  if (!archium_config_init()) {
    return "";
  }

  return preferred_package_manager;
}

int archium_config_set_preference(const char *key, const char *value) {
  if (!archium_config_init()) {
    return 0;
  }

  if (!validate_preference_key_value(key, value)) {
    archium_report_error(ARCHIUM_ERROR_CONFIG_INVALID,
                         "Invalid preference key or value", key);
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

  apply_preference_to_runtime(key, value);

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

  if (!key || key[0] == '\0' || strlen(key) >= CONFIG_KEY_MAX_LEN) {
    return NULL;
  }

  FILE *fp = fopen(preference_file_path, "r");
  if (!fp) {
    return NULL;
  }

  char line[CONFIG_BUFFER_SIZE];
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';
    if (!validate_preference_line(line)) {
      continue;
    }
    if (strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == '=') {
      fclose(fp);
      return strdup(line + strlen(key) + 1);
    }
  }

  fclose(fp);
  return NULL;
}

int archium_config_export_preferences(const char *file_path) {
  if (!archium_config_init() || !file_path || file_path[0] == '\0') {
    return 0;
  }

  FILE *dest = fopen(file_path, "w");
  if (!dest) {
    return 0;
  }

  FILE *src = fopen(preference_file_path, "r");
  if (!src) {
    fprintf(dest, "# Archium preferences export\n");
    fclose(dest);
    return 1;
  }

  char line[CONFIG_BUFFER_SIZE];
  while (fgets(line, sizeof(line), src)) {
    fputs(line, dest);
  }

  fclose(src);
  fclose(dest);
  return 1;
}

int archium_config_import_preferences(const char *file_path) {
  if (!archium_config_init() || !file_path || file_path[0] == '\0') {
    return 0;
  }

  FILE *src = fopen(file_path, "r");
  if (!src) {
    return 0;
  }

  char temp_file[CONFIG_BUFFER_SIZE];
  if (snprintf(temp_file, sizeof(temp_file), "%s.import.tmp",
               preference_file_path) >= (int)sizeof(temp_file)) {
    fclose(src);
    return 0;
  }

  FILE *temp = fopen(temp_file, "w");
  if (!temp) {
    fclose(src);
    return 0;
  }

  char line[CONFIG_BUFFER_SIZE];
  int valid = 1;
  while (fgets(line, sizeof(line), src)) {
    line[strcspn(line, "\n")] = '\0';

    if (!validate_preference_line(line)) {
      valid = 0;
      break;
    }

    fprintf(temp, "%s\n", line);
  }

  fclose(src);
  fclose(temp);

  if (!valid || rename(temp_file, preference_file_path) != 0) {
    unlink(temp_file);
    return 0;
  }

  apply_preference_if_exists("json_output");
  apply_preference_if_exists("batch_mode");
  apply_preference_if_exists("package_manager");
  apply_preference_if_exists("use_native_output");
  apply_preference_if_exists("show_welcome");
  apply_preference_if_exists("show_tips");
  apply_preference_if_exists("cache_ttl_seconds");
  apply_environment_overrides();

  return 1;
}

int archium_config_backup_preferences(char *out_path, size_t out_size) {
  if (!archium_config_init() || !out_path || out_size == 0) {
    return 0;
  }

  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);

  char stamp[32];
  if (strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tm_now) == 0) {
    return 0;
  }

  char backup_path[CONFIG_BUFFER_SIZE];
  if (snprintf(backup_path, sizeof(backup_path), "%s/preferences.backup.%s",
               config_dir_path, stamp) >= (int)sizeof(backup_path)) {
    return 0;
  }

  if (!copy_file_contents(preference_file_path, backup_path)) {
    FILE *empty = fopen(backup_path, "w");
    if (!empty) {
      return 0;
    }
    fclose(empty);
  }

  snprintf(out_path, out_size, "%s", backup_path);
  return 1;
}

int archium_config_restore_preferences(const char *file_path) {
  return archium_config_import_preferences(file_path);
}

void archium_config_print_effective(FILE *out) {
  FILE *target = out ? out : stdout;
  fprintf(target, "Effective configuration:\n");
  fprintf(target, "  package_manager=%s\n",
          preferred_package_manager[0] ? preferred_package_manager : "auto");
  fprintf(target, "  json_output=%d\n", config.json_output);
  fprintf(target, "  batch_mode=%d\n", config.batch_mode);
  fprintf(target, "  use_native_output=%d\n", config.use_native_output);
  fprintf(target, "  show_welcome=%d\n", config.show_welcome);
  fprintf(target, "  show_tips=%d\n", config.show_tips);
  fprintf(target, "  cache_ttl_seconds=%d\n", config.cache_ttl_seconds);
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
