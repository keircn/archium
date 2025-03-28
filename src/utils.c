#include "archium.h"

void handle_signal(int signal) {
  if (signal == SIGINT) {
    printf("\nInterrupt received. Exiting gracefully.\n");
    exit(EXIT_SUCCESS);
  }
}

void get_input(char *input, const char *prompt) {
  char *line = readline(prompt);
  if (line) {
    strcpy(input, line);
    free(line);
  } else {
    input[0] = '\0';
  }

  char *start = input;
  while (isspace((unsigned char)*start)) start++;
  memmove(input, start, strlen(start) + 1);
  char *end = input + strlen(input) - 1;
  while (end > input && isspace((unsigned char)*end)) end--;
  end[1] = '\0';

  if (strlen(input) == 0) {
    get_input(input, prompt);
  }
}

int is_valid_command(const char *command) {
  const char *valid_commands[] = {"u", "i", "r", "p",  "c",  "o",  "s", "h",
                                  "q", "l", "?", "cu", "dt", "cc", "lo"};
  int num_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(command, valid_commands[i]) == 0 ||
        strncmp(command, "u ", 2) == 0) {
      return 1;
    }
  }
  return 0;
}
