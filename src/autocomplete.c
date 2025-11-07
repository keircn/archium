#include "include/archium.h"

#define CACHE_TTL_SECONDS 3600
#define PACKAGE_CACHE_FILE "packages.cache"

static int is_cache_valid(const char *cache_path) {
  struct stat cache_stat;
  if (stat(cache_path, &cache_stat) != 0) {
    return 0;
  }

  time_t now = time(NULL);
  return (now - cache_stat.st_mtime) < CACHE_TTL_SECONDS;
}

static void load_cache_from_file(const char *cache_path) {
  FILE *fp = fopen(cache_path, "r");
  if (!fp) {
    return;
  }

  char line[256];
  int command_count = 0;

  while (fgets(line, sizeof(line), fp) != NULL) {
    size_t len = strcspn(line, "\n");
    if (len < sizeof(line)) {
      line[len] = 0;
    } else {
      line[sizeof(line) - 1] = 0;
    }
    if (strlen(line) == 0) continue;

    command_count++;
    cached_commands =
        realloc(cached_commands, sizeof(char *) * (command_count + 1));
    cached_commands[command_count - 1] = strdup(line);
  }

  fclose(fp);

  const char *custom_cmds[] = {"check", "info", "s"};
  for (int i = 0; i < 3; i++) {
    command_count++;
    cached_commands =
        realloc(cached_commands, sizeof(char *) * (command_count + 1));
    cached_commands[command_count - 1] = strdup(custom_cmds[i]);
  }

  if (cached_commands) {
    cached_commands[command_count] = NULL;
  }
}

static void save_cache_to_file(const char *cache_path) {
  FILE *fp = fopen(cache_path, "w");
  if (!fp) {
    return;
  }

  for (int i = 0; cached_commands && cached_commands[i] != NULL; i++) {
    if (strcmp(cached_commands[i], "check") != 0 &&
        strcmp(cached_commands[i], "info") != 0 &&
        strcmp(cached_commands[i], "s") != 0) {
      fprintf(fp, "%s\n", cached_commands[i]);
    }
  }

  fclose(fp);
}

void cache_pacman_commands(void) {
  if (cached_commands) {
    return;
  }

  const char *cache_dir = archium_config_get_cache_dir();
  if (!cache_dir) {
    fprintf(stderr, "\033[1;31mError: Failed to get cache directory.\033[0m\n");
    return;
  }

  char cache_path[512];
  if (snprintf(cache_path, sizeof(cache_path), "%s/%s", cache_dir,
               PACKAGE_CACHE_FILE) >= (int)sizeof(cache_path)) {
    fprintf(stderr, "\033[1;31mError: Cache path too long.\033[0m\n");
    return;
  }

  if (is_cache_valid(cache_path)) {
    load_cache_from_file(cache_path);
    if (cached_commands) {
      log_debug("Loaded package list from cache");
      return;
    }
  }

  FILE *fp;
  char path[1035];
  int command_count = 0;

  fp = popen("pacman -Ssq", "r");
  if (fp == NULL) {
    fprintf(stderr, "\033[1;31mFailed to run command\033[0m\n");
    return;
  }

  while (fgets(path, sizeof(path), fp) != NULL) {
    size_t len = strcspn(path, "\n");
    if (len < sizeof(path)) {
      path[len] = 0;
    } else {
      path[sizeof(path) - 1] = 0;
    }
    command_count++;
    cached_commands =
        realloc(cached_commands, sizeof(char *) * (command_count + 1));
    cached_commands[command_count - 1] = strdup(path);
  }

  pclose(fp);

  const char *custom_cmds[] = {"check", "info", "s"};
  for (int i = 0; i < 3; i++) {
    command_count++;
    cached_commands =
        realloc(cached_commands, sizeof(char *) * (command_count + 1));
    cached_commands[command_count - 1] = strdup(custom_cmds[i]);
  }

  if (cached_commands) {
    cached_commands[command_count] = NULL;
  }

  save_cache_to_file(cache_path);
  log_debug("Generated and cached package list");
}

char *command_generator(const char *text, int state) {
  static int list_index, len;

  if (!cached_commands) {
    cache_pacman_commands();
  }

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while (cached_commands && cached_commands[list_index]) {
    const char *name = cached_commands[list_index];
    list_index++;
    if (strncmp(name, text, len) == 0) {
      return strdup(name);
    }
  }

  return NULL;
}

char **command_completion(const char *text, int start, int end) {
  (void)start;
  (void)end;
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, command_generator);
}

void invalidate_package_cache(void) {
  const char *cache_dir = archium_config_get_cache_dir();
  if (!cache_dir) {
    return;
  }

  char cache_path[512];
  if (snprintf(cache_path, sizeof(cache_path), "%s/%s", cache_dir,
               PACKAGE_CACHE_FILE) >= (int)sizeof(cache_path)) {
    return;
  }

  unlink(cache_path);
  log_debug("Invalidated package cache");
}

void cleanup_cached_commands(void) {
  if (cached_commands) {
    for (int i = 0; cached_commands[i] != NULL; i++) {
      free(cached_commands[i]);
    }
    free(cached_commands);
    cached_commands = NULL;
  }
}
