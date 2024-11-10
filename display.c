#include "archium.h"

void display_version(void) {
    int pm_check = check_package_manager();
    const char *package_manager;
    switch (pm_check) {
        case 1:
            package_manager = "yay";
            break;
        case 2:
            package_manager = "paru";
            break;
        case 3:
            package_manager = "pacman";
            break;
        default:
            package_manager = "none";
    }

    char *pm_version = get_package_manager_version(package_manager);

    printf("\033[1;36m    __     Archium v1.4 - Fast & easy package management for Arch Linux\n"
           " .:--.'.   Written in C, powered by YAY, Paru, and Pacman.\n"
           "/ |   \\ |  %s %s\n"
           "`\" __ | |  \n"
           " .'.''| |  \n"
           "/ /   | |_ \033[0mThis program may be freely redistributed under the terms of the GNU General Public License.\n"
           "\033[1;36m\\ \\._,\\ '/ \033[0mCreated & maintained by Keiran\n"
           "\033[1;36m `--'  `\"  \033[0mWith enhancements by the community\n", 
           package_manager, pm_version);
    free(pm_version);
}

void display_help(void) {
    printf("\033[1;33mAvailable commands:\033[0m\n");
    printf("\033[1;32mu\033[0m [package] - Update system or specific package\n");
    printf("\033[1;32mi\033[0m           - Install packages\n");
    printf("\033[1;32mr\033[0m           - Remove packages\n");
    printf("\033[1;32mp\033[0m           - Purge packages\n");
    printf("\033[1;32mc\033[0m           - Clean cache\n");
    printf("\033[1;32mcc\033[0m          - Clear package build cache\n");
    printf("\033[1;32mo\033[0m           - Clean orphaned packages\n");
    printf("\033[1;32mlo\033[0m          - List orphaned packages\n");
    printf("\033[1;32ms\033[0m           - Search for packages\n");
    printf("\033[1;32ml\033[0m           - List installed packages\n");
    printf("\033[1;32m?\033[0m           - Show package information\n");
    printf("\033[1;32mdt\033[0m          - Display package dependency tree\n");
    printf("\033[1;32mcu\033[0m          - Check for updates\n");
    printf("\033[1;32mh\033[0m           - Help\n");
    printf("\033[1;32mq\033[0m           - Quit\n");
}
