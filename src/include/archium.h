#ifndef ARCHIUM_H
#define ARCHIUM_H

#include <stdio.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifndef ARCHIUM_VERSION
#define ARCHIUM_VERSION "1.10.1"
#endif

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 1024
#define MAX_RETRIES 3
#define TIMEOUT_SECONDS 30
#define ARCHIUM_REPO_URL "https://codeberg.org/keys/archium"

int sanitize_shell_input(const char *input, char *output, size_t output_size);
int validate_package_name(const char *package);
int validate_file_path(const char *path);

#include "autocomplete.h"
#include "commands.h"
#include "config.h"
#include "display.h"
#include "error.h"
#include "package_manager.h"
#include "plugin.h"
#include "utils.h"

#endif
