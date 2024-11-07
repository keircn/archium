#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <time.h>

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define LOG_FILE_PATH "/var/log/archie.log"

// Function Prototypes
void log_action(const char *action);
void handle_signal(int signal);
int check_archie_file();
int check_package_manager();
int check_git();
void install_git();
void install_yay();
void update_system(const char *package_manager);
void install_package(const char *package_manager, const char *packages);
void remove_package(const char *package_manager, const char *packages);
void purge_package(const char *package_manager, const char *packages);
void clean_cache(const char *package_manager);
void clean_orphans(const char *package_manager);
void search_package(const char *package_manager, const char *package);
void display_help();
void prompt_install_yay();
void get_input(char *input, const char *prompt);
int is_valid_command(char command);
void handle_command(const char *input, const char *package_manager);
void handle_exec_command(const char *command, const char *package_manager);
void display_version();
char **get_pacman_commands();
char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);
char *get_package_manager_version(const char *package_manager);
void cache_pacman_commands();

// Global Variables
char **cached_commands = NULL;

void log_action(const char *action) {
    FILE *log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "%s: %s\n", ctime(&now), action);
        fclose(log_file);
    }
}

void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nInterrupt received. Exiting gracefully.\n");
        exit(0);
    }
}

int check_archie_file() {
    const char *home = getenv("HOME");
    char path[MAX_INPUT_LENGTH];
    snprintf(path, sizeof(path), "%s/.archie-use-paru", home);
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int check_package_manager() {
    if (check_archie_file()) {
        return 2;  // paru
    }
    if (system("command -v yay > /dev/null 2>&1") == 0) {
        return 1;  // yay
    }
    if (system("command -v paru > /dev/null 2>&1") == 0) {
        return 2;  // paru
    }
    if (system("command -v pacman > /dev/null 2>&1") == 0) {
        return 3;  // pacman
    }
    return 0;  // none
}

int check_git() {
    return system("command -v git > /dev/null 2>&1") == 0;
}

void install_git() {
    printf("Installing git...\n");
    system("sudo pacman -S --noconfirm git");
}

void install_yay() {
    printf("Installing yay...\n");
    system("mkdir -p $HOME/.cache/archie/made-by-gurov && "
           "cd $HOME/.cache/archie/made-by-gurov && "
           "git clone https://aur.archlinux.org/yay-bin.git && "
           "cd yay-bin && "
           "makepkg -scCi && "
           "cd && "
           "rm -rf $HOME/.cache/archie/");
    printf("Installation of yay is complete. Please restart your shell and relaunch the script.\n");
}

void update_system(const char *package_manager) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
    system(command);
    log_action("System updated");
}

void install_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -S %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Installed packages: %s", packages);
    log_action(log_entry);
}

void remove_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -R %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Removed packages: %s", packages);
    log_action(log_entry);
}

void purge_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Rns %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Purged packages: %s", packages);
    log_action(log_entry);
}

void clean_cache(const char *package_manager) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Sc --noconfirm", package_manager);
    system(command);
    log_action("Cache cleaned");
}

void clean_orphans(const char *package_manager) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Rns $(pacman -Qdtq)", package_manager);
    system(command);
    log_action("Orphan packages cleaned");
}

void search_package(const char *package_manager, const char *package) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Ss %s", package_manager, package);
    system(command);
}

void display_help() {
    printf("Available options are:\n");
    printf("u - Update the system\n");
    printf("i - Install packages (space-separated for multiple packages)\n");
    printf("r - Remove packages (space-separated for multiple packages)\n");
    printf("p - Purge packages (space-separated for multiple packages)\n");
    printf("c - Clean cache\n");
    printf("o - Clean orphaned packages\n");
    printf("s - Search for a package\n");
    printf("h - Help\n");
    printf("q - Quit\n");
}

void prompt_install_yay() {
    char response[10];
    printf("Error: No suitable package manager is installed.\n");
    sleep(1);
    printf("Do you want to install yay? (y/n): ");
    scanf("%s", response);

    if (strcmp(response, "y") == 0 || strcmp(response, "yes") == 0) {
        if (!check_git()) {
            install_git();
        }
        install_yay();
    } else {
        printf("Exiting the program.\n");
        exit(0);
    }
}

void get_input(char *input, const char *prompt) {
    char *line = readline(prompt);
    if (line) {
        strcpy(input, line);
        free(line);
    } else {
        input[0] = '\0';  // Handle EOF
    }

    // Remove newline character
    input[strcspn(input, "\n")] = 0;

    if (strlen(input) == 0) {
        get_input(input, prompt);
    }
}

int is_valid_command(char command) {
    return command == 'u' || command == 'i' || command == 'r' ||
           command == 'p' || command == 'c' || command == 'o' ||
           command == 's' || command == 'h' || command == 'q';
}

void handle_command(const char *input, const char *package_manager) {
    char choice = input[0];
    if (strlen(input) == 1 && is_valid_command(choice)) {
        switch (choice) {
            case 'u':
                update_system(package_manager);
                break;
            case 'i': {
                char packages[MAX_INPUT_LENGTH];
                rl_attempted_completion_function = command_completion;
                get_input(packages, "Enter package names to install: ");
                rl_attempted_completion_function = NULL;
                install_package(package_manager, packages);
                break;
            }
            case 'r': {
                char packages[MAX_INPUT_LENGTH];
                rl_attempted_completion_function = command_completion;
                get_input(packages, "Enter package names to remove: ");
                rl_attempted_completion_function = NULL;
                remove_package(package_manager, packages);
                break;
            }
            case 'p': {
                char packages[MAX_INPUT_LENGTH];
                rl_attempted_completion_function = command_completion;
                get_input(packages, "Enter package names to purge: ");
                rl_attempted_completion_function = NULL;
                purge_package(package_manager, packages);
                break;
            }
            case 'c':
                clean_cache(package_manager);
                break;
            case 'o':
                clean_orphans(package_manager);
                break;
            case 's': {
                char package[MAX_INPUT_LENGTH];
                get_input(package, "Enter package name to search: ");
                search_package(package_manager, package);
                break;
            }
            case 'h':
                display_help();
                break;
            case 'q':
                exit(0);
                break;
            default:
                printf("Invalid option.\n");
                break;
        }
    } else {
        printf("Invalid input. Please input a valid command.\n");
        display_help();
    }
}

void handle_exec_command(const char *command, const char *package_manager) {
    if (strcmp(command, "u") == 0) {
        update_system(package_manager);
    } else if (strcmp(command, "i") == 0) {
        char packages[MAX_INPUT_LENGTH];
        get_input(packages, "Enter package names to install: ");
        install_package(package_manager, packages);
    } else if (strcmp(command, "r") == 0) {
        char packages[MAX_INPUT_LENGTH];
        get_input(packages, "Enter package names to remove: ");
        remove_package(package_manager, packages);
    } else if (strcmp(command, "p") == 0) {
        char packages[MAX_INPUT_LENGTH];
        get_input(packages, "Enter package names to purge: ");
        purge_package(package_manager, packages);
    } else if (strcmp(command, "c") == 0) {
        clean_cache(package_manager);
    } else if (strcmp(command, "o") == 0) {
        clean_orphans(package_manager);
    } else if (strcmp(command, "s") == 0) {
        char package[MAX_INPUT_LENGTH];
        get_input(package, "Enter package name to search: ");
        search_package(package_manager, package);
    } else if (strcmp(command, "h") == 0) {
        display_help();
    } else {
        printf("Invalid command for --exec: %s\n", command);
    }
}

char *get_package_manager_version(const char *package_manager) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s --version", package_manager);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return strdup("unknown");
    }

    char version[COMMAND_BUFFER_SIZE];
    if (fgets(version, sizeof(version), fp) != NULL) {
        version[strcspn(version, "\n")] = '\0';  // Remove newline character
    } else {
        strcpy(version, "unknown");
    }
    pclose(fp);

    // Remove the package manager name from the version string
    char *version_start = strstr(version, " ");
    if (version_start != NULL) {
        version_start++;
    } else {
        version_start = version;
    }

    return strdup(version_start);
}

void display_version() {
    int pm_check = check_package_manager();
    const char *package_manager;
    if (pm_check == 1) {
        package_manager = "yay";
    } else if (pm_check == 2) {
        package_manager = "paru";
    } else if (pm_check == 3) {
        package_manager = "pacman";
    } else {
        package_manager = "none";
    }

    char *pm_version = get_package_manager_version(package_manager);

    printf("    __     Archie v1.4 - Fast & easy package management for Arch Linux\n"
           " .:--.'.   Written in C, powered by YAY, Paru, and Pacman.\n"
           "/ |   \\ |  %s %s\n"  // Package manager and version
           "`\" __ | |  \n"
           " .'.''| |  \n"
           "/ /   | |_ This program may be freely redistributed under the terms of the GNU General Public License.\n"
           "\\ \\._,\\ '/ Created & maintained by Gurov\n"
           " `--'  `\"  With enhancements by the community\n", package_manager, pm_version);
    free(pm_version);
}

void cache_pacman_commands() {
    if (cached_commands) {
        return;  // Already cached
    }

    FILE *fp;
    char path[1035];
    int command_count = 0;

    // Execute the pacman -Ssq command
    fp = popen("pacman -Ssq", "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        exit(1);
    }

    // Read the output a line at a time and store it in the commands array
    while (fgets(path, sizeof(path), fp) != NULL) {
        command_count++;
        cached_commands = realloc(cached_commands, sizeof(char *) * (command_count + 1));
        path[strcspn(path, "\n")] = 0;  // Remove newline character
        cached_commands[command_count - 1] = strdup(path);
    }

    // Close the file
    pclose(fp);

    // Null-terminate the array
    if (cached_commands) {
        cached_commands[command_count] = NULL;
    }
}

char *command_generator(const char *text, int state) {
    static int list_index, len;

    if (!cached_commands) {
        cache_pacman_commands();
    }

    // Initialize on first call
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    // Return the next match from the command list
    while (cached_commands && cached_commands[list_index]) {
        const char *name = cached_commands[list_index];
        list_index++;
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    // If no more matches, return NULL
    return NULL;
}

char **command_completion(const char *text, int start, int end) {
    (void)start;
    (void)end;
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        display_version();
        return 0;
    }

    if (argc > 2 && strcmp(argv[1], "--exec") == 0) {
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

        handle_exec_command(argv[2], package_manager);
        return 0;
    } else if (argc == 2 && strcmp(argv[1], "--exec") == 0) {
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

        char command[MAX_INPUT_LENGTH];
        get_input(command, "Enter command to execute (u, i, r, p, c, o, s, h): ");
        handle_exec_command(command, package_manager);
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

    cache_pacman_commands();

    printf("Welcome to Archie v1.4, type \"h\" for help\n");

    while (1) {
        char input_line[MAX_INPUT_LENGTH];
        get_input(input_line, "$ ");
        if (*input_line) {
            add_history(input_line);
            handle_command(input_line, package_manager);
        }
    }

    return 0;
}