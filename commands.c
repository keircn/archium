#include "archium.h"

void handle_command(const char *input, const char *package_manager) {
    if (is_valid_command(input)) {
        if (strcmp(input, "u") == 0) {
            update_system(package_manager, NULL);
        } else if (strncmp(input, "u ", 2) == 0) {
            char package[MAX_INPUT_LENGTH];
            strcpy(package, input + 2);
            update_system(package_manager, package);
        } else if (strcmp(input, "i") == 0) {
            char packages[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(packages, "Enter package names to install: ");
            rl_attempted_completion_function = NULL;
            install_package(package_manager, packages);
        } else if (strcmp(input, "r") == 0) {
            char packages[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(packages, "Enter package names to remove: ");
            rl_attempted_completion_function = NULL;
            remove_package(package_manager, packages);
        } else if (strcmp(input, "p") == 0) {
            char packages[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(packages, "Enter package names to purge: ");
            rl_attempted_completion_function = NULL;
            purge_package(package_manager, packages);
        } else if (strcmp(input, "c") == 0) {
            clean_cache(package_manager);
        } else if (strcmp(input, "ccache") == 0) {
            clear_build_cache();
        } else if (strcmp(input, "o") == 0) {
            clean_orphans(package_manager);
        } else if (strcmp(input, "lo") == 0) {
            list_orphans();
        } else if (strcmp(input, "s") == 0) {
            char package[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(package, "Enter package name to search: ");
            rl_attempted_completion_function = NULL;
            search_package(package_manager, package);
        } else if (strcmp(input, "l") == 0) {
            list_installed_packages();
        } else if (strcmp(input, "info") == 0) {
            char package[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(package, "Enter package name to show info: ");
            rl_attempted_completion_function = NULL;
            show_package_info(package_manager, package);
        } else if (strcmp(input, "check") == 0) {
            check_package_updates();
        } else if (strcmp(input, "dep") == 0) {
            char package[MAX_INPUT_LENGTH];
            rl_attempted_completion_function = command_completion;
            get_input(package, "Enter package name to view dependencies: ");
            rl_attempted_completion_function = NULL;
            display_dependency_tree(package_manager, package);
        } else if (strcmp(input, "h") == 0) {
            display_help();
        } else if (strcmp(input, "q") == 0) {
            printf("\033[1;31mExiting Archium.\033[0m\n");
            exit(0);
        } else {
            printf("\033[1;31mInvalid option.\033[0m\n");
        }
    } else {
        printf("\033[1;31mInvalid input. Please input a valid command.\033[0m\n");
        display_help();
    }
}

void handle_exec_command(const char *command, const char *package_manager) {
    if (is_valid_command(command)) {
        handle_command(command, package_manager);
    } else {
        printf("\033[1;31mInvalid command for --exec: %s\033[0m\n", command);
    }
}

void update_system(const char *package_manager, const char *package) {
    printf("\033[1;34mUpdating...\033[0m\n");
    char command[COMMAND_BUFFER_SIZE];
    if (package) {
        snprintf(command, sizeof(command), "%s -S %s", package_manager, package);
        printf("\033[1;34mUpgrading package: %s\033[0m\n", package);
    } else {
        snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
        printf("\033[1;34mUpgrading system...\033[0m\n");
    }
    system(command);
    log_action("System updated");
}

void clear_build_cache() {
    printf("\033[1;34mClearing package build cache...\033[0m\n");
    system("rm -rf $HOME/.cache/yay");
    log_action("Cleared package build cache");
}

void list_orphans() {
    printf("\033[1;34mListing orphaned packages...\033[0m\n");
    system("pacman -Qdt");
}

void display_dependency_tree(const char *package_manager, const char *package) {
    printf("\033[1;34mDisplaying dependency tree for package: %s\033[0m\n", package);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "pactree %s", package);
    system(command);
}