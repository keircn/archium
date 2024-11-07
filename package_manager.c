#include "archium.h"

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