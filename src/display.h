#ifndef DISPLAY_H
#define DISPLAY_H

void display_version(void);
void display_cli_help(void);
void display_help(void);
void display_help_category(const char *category);
void display_help_quick(void);
void display_help_command(const char *command);
void display_random_tip(void);
void handle_signal(int signal);
void show_progress_bar(int current, int total, const char *prefix);
void show_spinner(int position, const char *message);
void parse_and_show_upgrade_result(const char *output, int exit_code);
void parse_and_show_install_result(const char *output, int exit_code,
                                   const char *package);
void parse_and_show_remove_result(const char *output, int exit_code,
                                  const char *package);
void parse_and_show_generic_result(const char *output, int exit_code,
                                   const char *operation);

#endif