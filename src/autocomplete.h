#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

extern char **cached_commands;

char **command_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);
void cache_pacman_commands(void);
void cleanup_cached_commands(void);

#endif