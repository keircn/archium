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

#include "version.h"

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define MAX_RETRIES 3
#define TIMEOUT_SECONDS 30
#define ARCHIUM_REPO_URL "https://github.com/keircn/archium.git"

#include "autocomplete.h"
#include "commands.h"
#include "config.h"
#include "display.h"
#include "error.h"
#include "package_manager.h"
#include "plugin.h"
#include "utils.h"

#endif
