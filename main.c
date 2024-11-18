#include "archium.h"

char **cached_commands = NULL;

int main(int argc, char *argv[]) {
    ArchiumError status = ARCHIUM_SUCCESS;
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGABRT, handle_signal);

    log_info("Archium started");

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        display_version();
        return ARCHIUM_SUCCESS;
    }

    if (geteuid() != 0 && access("/usr/bin/sudo", X_OK) != 0) {
        log_error("Insufficient privileges and sudo not available", ARCHIUM_ERROR_PERMISSION);
        fprintf(stderr, "\033[1;31mError: This operation requires root privileges.\033[0m\n");
        return ARCHIUM_ERROR_PERMISSION;
    }

    const char *package_manager;
    int pm_check = check_package_manager();
    
    switch (pm_check) {
        case 1:
            package_manager = "yay";
            log_info("Using package manager: yay");
            break;
        case 2:
            package_manager = "paru";
            log_info("Using package manager: paru");
            break;
        case 3:
            package_manager = "pacman";
            log_info("Using package manager: pacman");
            break;
        default:
            log_error("No supported package manager found", ARCHIUM_ERROR_PACKAGE_MANAGER);
            prompt_install_yay();
            return ARCHIUM_ERROR_PACKAGE_MANAGER;
    }

    rl_attempted_completion_function = command_completion;
    cache_pacman_commands();

    if (argc > 2 && strcmp(argv[1], "--exec") == 0) {
        status = handle_exec_command(argv[2], package_manager);
        log_info("Executed command in exec mode");
        return status;
    } else if (argc == 2 && strcmp(argv[1], "--exec") == 0) {
        char command[MAX_INPUT_LENGTH];
        get_input(command, "\033[1;36mEnter command to execute (type 'h' for help): \033[0m");
        status = handle_exec_command(command, package_manager);
        log_info("Executed interactive command in exec mode");
        return status;
    }

    printf("\033[1;36m╔════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║      Welcome to Archium v1.0.0         ║\033[0m\n");
    printf("\033[1;36m║      Type \"h\" for help                 ║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════╝\033[0m\n");

    while (1) {
        char input_line[MAX_INPUT_LENGTH];
        get_input(input_line, "\033[1;32mArchium $ \033[0m");
        
        if (*input_line) {
            add_history(input_line);
            status = handle_command(input_line, package_manager);
            
            if (status != ARCHIUM_SUCCESS) {
                handle_error(status, "Command execution failed");
                log_error(input_line, status);
            }
        }
    }

    return ARCHIUM_SUCCESS;
}
