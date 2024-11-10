#include "archium.h"

void cache_pacman_commands() {
    if (cached_commands) {
        return;
    }

    FILE *fp;
    char path[1035];
    int command_count = 0;

    fp = popen("pacman -Ssq", "r");
    if (fp == NULL) {
        printf("\033[1;31mFailed to run command\033[0m\n");
        exit(1);
    }

    while (fgets(path, sizeof(path), fp) != NULL) {
        command_count++;
        cached_commands = realloc(cached_commands, sizeof(char *) * (command_count + 1));
        path[strcspn(path, "\n")] = 0;
        cached_commands[command_count - 1] = strdup(path);
    }

    pclose(fp);

    const char *custom_cmds[] = {"check", "info", "s"};
    for (int i = 0; i < 3; i++) {
        command_count++;
        cached_commands = realloc(cached_commands, sizeof(char *) * (command_count + 1));
        cached_commands[command_count - 1] = strdup(custom_cmds[i]);
    }

    if (cached_commands) {
        cached_commands[command_count] = NULL;
    }
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
