#include "archium.h"

char **cached_commands = NULL;

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        display_version();
        return 0;
    }

    const char *package_manager;
    int pm_check = check_package_manager();

    if (pm_check == 1) {
        package_manager = "yay";
    } else if (pm_check == 2) {
        package_manager = "paru";
    } else if (pm_check == 3) {
        package_manager = "pacman";
    } else {
        prompt_install_yay();
        return 1;
    }

    if (argc > 2 && strcmp(argv[1], "--exec") == 0) {
        handle_exec_command(argv[2], package_manager);
        return 0;
    } else if (argc == 2 && strcmp(argv[1], "--exec") == 0) {
        char command[MAX_INPUT_LENGTH];
        get_input(command, "Enter command to execute (type 'h' for help): ");
        handle_exec_command(command, package_manager);
        return 0;
    }

    cache_pacman_commands();

    printf("\033[1;36mWelcome to Archium, type \"h\" for help\033[0m\n");

    while (1) {
        char input_line[MAX_INPUT_LENGTH];
        get_input(input_line, "\033[1;32mArchium $ \033[0m");
        if (*input_line) {
            add_history(input_line);
            handle_command(input_line, package_manager);
        }
    }

    return 0;
}
