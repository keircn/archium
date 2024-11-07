#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 256
#define COMMAND_BUFFER_SIZE 512
#define LOG_FILE_PATH "/var/log/archium.log"

// Function Prototypes
void log_action(const char *action);
void handle_signal(int signal);
int check_archium_file();
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
void list_installed_packages();
void show_package_info(const char *package_manager, const char *package);
void check_package_updates();
void display_help();
void prompt_install_yay();
void get_input(char *input, const char *prompt);
int is_valid_command(const char *command);
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

int check_archium_file() {
    const char *home = getenv("HOME");
    char path[MAX_INPUT_LENGTH];
    snprintf(path, sizeof(path), "%s/.archium-use-paru", home);
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int check_package_manager() {
    if (check_archium_file()) {
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
    printf("\033[1;32mInstalling git...\033[0m\n");
    system("sudo pacman -S --noconfirm git");
}

void install_yay() {
    printf("\033[1;32mInstalling yay...\033[0m\n");
    system("mkdir -p $HOME/.cache/archium/setup && "
           "cd $HOME/.cache/archium/setup && "
           "git clone https://aur.archlinux.org/yay-bin.git && "
           "cd yay-bin && "
           "makepkg -scCi && "
           "cd && "
           "rm -rf $HOME/.cache/archium/");
    printf("\033[1;32mInstallation of yay is complete. Please restart your shell and relaunch Archium.\033[0m\n");
}

void update_system(const char *package_manager) {
    printf("\033[1;34mUpdating system...\033[0m\n");
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
    system(command);
    log_action("System updated");
}

void install_package(const char *package_manager, const char *packages) {
    printf("\033[1;34mInstalling packages: %s\033[0m\n", packages);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -S %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Installed packages: %s", packages);
    log_action(log_entry);
}

void remove_package(const char *package_manager, const char *packages) {
    printf("\033[1;34mRemoving packages: %s\033[0m\n", packages);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -R %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Removed packages: %s", packages);
    log_action(log_entry);
}

void purge_package(const char *package_manager, const char *packages) {
    printf("\033[1;34mPurging packages: %s\033[0m\n", packages);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Rns %s", package_manager, packages);
    system(command);
    char log_entry[COMMAND_BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Purged packages: %s", packages);
    log_action(log_entry);
}

void clean_cache(const char *package_manager) {
    printf("\033[1;34mCleaning cache...\033[0m\n");
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Sc --noconfirm", package_manager);
    system(command);
    log_action("Cache cleaned");
}

void clean_orphans(const char *package_manager) {
    printf("\033[1;34mCleaning orphaned packages...\033[0m\n");
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Rns $(pacman -Qdtq)", package_manager);
    system(command);
    log_action("Orphan packages cleaned");
}

void search_package(const char *package_manager, const char *package) {
    printf("\033[1;34mSearching for package: %s\033[0m\n", package);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Ss %s", package_manager, package);
    system(command);
}

void list_installed_packages() {
    printf("\033[1;34mListing installed packages...\033[0m\n");
    system("pacman -Qe");
}

void show_package_info(const char *package_manager, const char *package) {
    printf("\033[1;34mShowing info for package: %s\033[0m\n", package);
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Qi %s", package_manager, package);
    system(command);
}

void check_package_updates() {
    printf("\033[1;34mChecking for package updates...\033[0m\n");
    system("pacman -Qu");
}

void display_help() {
    printf("\033[1;33mAvailable commands:\033[0m\n");
    printf("\033[1;32mu\033[0m    - Update the system\n");
    printf("\033[1;32mi\033[0m    - Install packages (space-separated for multiple packages)\n");
    printf("\033[1;32mr\033[0m    - Remove packages (space-separated for multiple packages)\n");
    printf("\033[1;32mp\033[0m    - Purge packages (space-separated for multiple packages)\n");
    printf("\033[1;32mc\033[0m    - Clean cache\n");
    printf("\033[1;32mo\033[0m    - Clean orphaned packages\n");
    printf("\033[1;32ms\033[0m    - Search for a package\n");
    printf("\033[1;32ml\033[0m    - List installed packages\n");
    printf("\033[1;32minfo\033[0m - Show package information\n");
    printf("\033[1;32mcheck\033[0m - Check for package updates\n");
    printf("\033[1;32mh\033[0m    - Help\n");
    printf("\033[1;32mq\033[0m    - Quit\n");
}

void prompt_install_yay() {
    char response[10];
    printf("\033[1;31mError: No suitable package manager is installed.\033[0m\n");
    sleep(1);
    printf("Do you want to install yay? (\033[1;32my\033[0m/\033[1;31mn\033[0m): ");
    scanf("%s", response);

    if (strcasecmp(response, "y") == 0 || strcasecmp(response, "yes") == 0) {
        if (!check_git()) {
            install_git();
        }
        install_yay();
    } else {
        printf("\033[1;31mExiting Archium.\033[0m\n");
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

    // Remove leading and trailing whitespace
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
    return strcmp(command, "u") == 0 || strcmp(command, "i") == 0 || strcmp(command, "r") == 0 ||
           strcmp(command, "p") == 0 || strcmp(command, "c") == 0 || strcmp(command, "o") == 0 ||
           strcmp(command, "s") == 0 || strcmp(command, "h") == 0 || strcmp(command, "q") == 0 ||
           strcmp(command, "l") == 0 || strcmp(command, "info") == 0 || strcmp(command, "check") == 0;
}

void handle_command(const char *input, const char *package_manager) {
    if (is_valid_command(input)) {
        if (strcmp(input, "u") == 0) {
            update_system(package_manager);
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
        } else if (strcmp(input, "o") == 0) {
            clean_orphans(package_manager);
        } else if (strcmp(input, "s") == 0) {
            char package[MAX_INPUT_LENGTH];
            get_input(package, "Enter package name to search: ");
            search_package(package_manager, package);
        } else if (strcmp(input, "l") == 0) {
            list_installed_packages();
        } else if (strcmp(input, "info") == 0) {
            char package[MAX_INPUT_LENGTH];
            get_input(package, "Enter package name to show info: ");
            show_package_info(package_manager, package);
        } else if (strcmp(input, "check") == 0) {
            check_package_updates();
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

    printf("\033[1;36m    __     Archium v1.5 - Fast & easy package management for Arch Linux\n"
           " .:--.'.   Written in C, powered by YAY, Paru, and Pacman.\n"
           "/ |   \\ |  %s %s\n"
           "`\" __ | |  \n"
           " .'.''| |  \n"
           "/ /   | |_ \033[0mThis program may be freely redistributed under the terms of the GNU General Public License.\n"
           "\033[1;36m\\ \\._,\\ '/ \033[0mCreated & maintained by Keiran\n"
           "\033[1;36m `--'  `\"  \033[0mWith enhancements by the community\n", package_manager, pm_version);
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
        printf("\033[1;31mFailed to run command\033[0m\n");
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

    printf("\033[1;36mWelcome to Archium v1.5, type \"h\" for help\033[0m\n");

    while (1) {
        char input_line[MAX_INPUT_LENGTH];
        get_input(input_line, "\033[1;32mArchium$ \033[0m");
        if (*input_line) {
            add_history(input_line);
            handle_command(input_line, package_manager);
        }
    }

    return 0;
}