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

  char build_time[6];
  strncpy(build_time, __TIME__, 5);
  build_time[5] = '\0';

  printf(
      "\033[1;34m"
      "                                                  \n"
      "                                                  \n"
      "                        ██                        \n"
      "                       ████                       \n"
      "                      ██████                      \n"
      "                     ███  ███                     \n"
      "                    ███    ███                    \n"
      "                   ███      ███                   \n"
      "                  ███        ███                  \n"
      "                 ████████     ███                 \n"
      "                ███   █████    ███                \n"
      "               ███       █████  ███               \n"
      "              ██████████████████████              \033[0m\n\n"
      "                                                  \n"
      "                                                  \n"
      "\033[1;36mArchium v%s\033[0m - \033[1;32mFast & easy package management "
      "for Arch Linux\033[0m\n"
      "\033[0mWritten in \033[1mC\033[0m and powered by \033[1mYAY\033[0m, "
      "\033[1mParu\033[0m, and \033[1mPacman\033[0m\n"
      "\033[0mPackage Manager: \033[1;36m%s\033[0m (Version: "
      "\033[1;36m%s\033[0m)\n"
      "\033[0mBuild Date: \033[1;33m%.3s%s %s\033[0m\n\n"
      "\033[0mThis program is subject to the terms of the \033[1;31mGPL "
      "License\033[0m\n"
      "\033[0mArchie originally created by \033[1;36mGurov\033[0m\n"
      "\033[0mArchium forked and maintained by \033[1;36mKeiran\033[0m\n\n"
      "\033[0mGitHub: \033[1;34mhttps://github.com/keircn/archium\033[0m\n",
      ARCHIUM_VERSION, package_manager, pm_version, __DATE__, &__DATE__[3],
      build_time);

  free(pm_version);
}

void display_cli_help(void) {
  printf("\n\033[1;33mCommand-line arguments:\033[0m\n");
  printf(
      "\033[1;32m--help\033[0m,    \033[1;32m-h\033[0m       - Display this "
      "help message\n");
  printf(
      "\033[1;32m--version\033[0m, \033[1;32m-v\033[0m       - Display "
      "version information\n");
  printf(
      "\033[1;32m--verbose\033[0m, \033[1;32m-V\033[0m       - Enable "
      "verbose logging\n");
  printf(
      "\033[1;32m--exec <command>\033[0m    - Execute a specific command "
      "directly\n");
  printf(
      "\033[1;32m--self-update\033[0m - Update Archium to the latest "
      "version\n");
  printf("\n\033[1;33mExample:\033[0m\n");
  printf("  \033[1;32marchium --exec u\033[0m - Update the system\n");
  printf("  \033[1;32marchium --exec i\033[0m - Install packages\n");
}

void display_help(void) {
  printf("\n");
  printf("\033[1;33mAvailable commands:\033[0m\n");
  printf("\033[1;32mu\033[0m [package] - Update system or specific package\n");
  printf("\033[1;32mi\033[0m           - Install packages\n");
  printf("\033[1;32mr\033[0m           - Remove packages\n");
  printf("\033[1;32mp\033[0m           - Purge packages\n");
  printf("\033[1;32mc\033[0m           - Clean cache\n");
  printf("\033[1;32mo\033[0m           - Clean orphaned packages\n");
  printf("\033[1;32ms\033[0m           - Search for packages\n");
  printf("\033[1;32ml\033[0m           - List installed packages\n");
  printf("\033[1;32m?\033[0m           - Show package information\n");
  printf("\033[1;32mcc\033[0m          - Clear package build cache\n");
  printf("\033[1;32mlo\033[0m          - List orphaned packages\n");
  printf("\033[1;32mdt\033[0m          - Display package dependency tree\n");
  printf("\033[1;32mcu\033[0m          - Check for updates\n");
  printf("\033[1;32msi\033[0m          - List installed packages by size\n");
  printf("\033[1;32mre\033[0m          - Show recently installed packages\n");
  printf("\033[1;32mex\033[0m          - List explicitly installed packages\n");
  printf("\033[1;32mow\033[0m          - Find which package owns a file\n");
  printf("\033[1;32mba\033[0m          - Backup pacman configuration\n");
  printf("\033[1;32mdt\033[0m          - Display package dependency tree\n");
  printf("\033[1;32mcu\033[0m          - Check for updates\n");
  printf("\033[1;32msi\033[0m          - List installed packages by size\n");
  printf("\033[1;32mre\033[0m          - Show recently installed packages\n");
  printf("\033[1;32mex\033[0m          - List explicitly installed packages\n");
  printf("\033[1;32mow\033[0m          - Find which package owns a file\n");
  printf("\033[1;32mba\033[0m          - Backup pacman configuration\n");
  printf("\033[1;32mh\033[0m           - Help\n");
  printf("\033[1;32mq\033[0m           - Quit\n");
}
