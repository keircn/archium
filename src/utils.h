#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void log_action(const char *action);
void log_debug(const char *debug_message);
void log_info(const char *info_message);
int execute_command_with_spinner(const char *command, const char *message);
int execute_command_with_output_capture(const char *command,
                                        const char *message,
                                        char *output_buffer,
                                        size_t buffer_size);

#endif