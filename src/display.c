#include <time.h>

#include "include/archium.h"

static const char *archium_tips[] = {
    "Use 'h' or 'help' to see all available commands.",
    "You can update a specific package with 'u <package>'.",
    "Use 's' to search for packages by name.",
    "'cc' clears the AUR helper build cache.",
    "'lo' lists orphaned packages you can remove.",
    "'dt' shows the dependency tree for a package.",
    "'ba' backs up your pacman.conf safely.",
    "Use tab completion for faster command entry.",
    "'ow' finds which package owns a file.",
    "'si' lists installed packages by size."};
#define NUM_TIPS (sizeof(archium_tips) / sizeof(archium_tips[0]))

void display_random_tip(void) {
  srand((unsigned int)time(NULL) ^ getpid());
  int idx = rand() % NUM_TIPS;
  printf("\033[1;35mTip:\033[0m %s\n", archium_tips[idx]);
}

void display_fallback_logo(void) {
  printf("\033[1;34m");
  printf("                     -@                \n");
  printf("                    .##@               \n");
  printf("                   .####@              \n");
  printf("                   @#####@             \n");
  printf("                 . *######@            \n");
  printf("                .##@o@#####@           \n");
  printf("               /############@          \n");
  printf("              /##############@         \n");
  printf("             @######@**%%%%######@        \n");
  printf("            @######`     %%#####o       \n");
  printf("           @######@       ######%%      \n");
  printf("         -@#######h       ######@.`    \n");
  printf("        /#####h**``       `**%%%%@####@   \n");
  printf("       @H@*`                    `*%%%%#@  \n");
  printf("      *`                            `* \n");
  printf("\033[0m\n\n");
}

void display_version(void) {
  FILE *file = fopen("version.txt", "r");
  if (file) {
    printf("\033[1;34m\n");
    char line[256];
    while (fgets(line, sizeof(line), file)) {
      printf("%s", line);
    }
    printf("\033[0m\n");
    fclose(file);
  } else {
    display_fallback_logo();
  }

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
      "\033[1;36mArchium v%s\033[0m - \033[1;32mFast & easy package management "
      "for Arch Linux\033[0m\n"
      "\033[0mWritten in \033[1mC\033[0m and powered by \033[1mYAY\033[0m, "
      "\033[1mParu\033[0m, and \033[1mPacman\033[0m\n"
      "\033[0mPackage Manager: \033[1;36m%s\033[0m (Version: "
      "\033[1;36m%s\033[0m)\n"
      "\033[0mBuild Date: \033[1;33m%.3s%s %s\033[0m\n\n"
      "\033[0mThis program is subject to the terms of the \033[1;31mMIT "
      "License\033[0m\n"
      "\033[0mArchie originally created by \033[1;36mGurov\033[0m\n"
      "\033[0mArchium forked and maintained by \033[1;36mKeiran\033[0m\n\n"
      "\033[0mGitHub: \033[1;34mhttps://codeberg.org/keys/archium\033[0m\n",
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
  printf(
      "  \033[1;32m--json\033[0m       - Output machine-readable JSON and "
      "suppress UI\n");
  printf(
      "  \033[1;32m--batch\033[0m      - Non-interactive batch mode (no "
      "prompts)\n");
  printf("\n\033[1;33mExample:\033[0m\n");
  printf("  \033[1;32marchium --exec u\033[0m - Update the system\n");
  printf("  \033[1;32marchium --exec i\033[0m - Install packages\n");
}

void display_help(void) {
  printf("\n\033[1;33mArchium Help\033[0m\n");
  printf("Use \033[1;32mh <category>\033[0m for specific help, or:\n\n");
  printf("\033[1;36mCategories:\033[0m\n");
  printf(
      "  \033[1;32mpackages\033[0m  - Package operations (install, remove, "
      "search)\n");
  printf(
      "  \033[1;32msystem\033[0m    - System utilities (health, maintenance, "
      "diagnostics)\n");
  printf(
      "  \033[1;32minfo\033[0m      - Information commands (list, show, "
      "dependencies)\n");
  printf("  \033[1;32mconfig\033[0m    - Configuration and plugins\n");
  printf("  \033[1;32mplugin\033[0m    - Plugin management commands\n\n");
  printf("\033[1;36mQuick Help:\033[0m\n");
  printf("  \033[1;32mh quick\033[0m   - Show abbreviated command list\n");
  printf("  \033[1;32mh tips\033[0m    - Show helpful tips\n");
  printf(
      "  \033[1;32mh <cmd>\033[0m   - Detailed help for specific command\n\n");
  printf("  \033[1;32mq\033[0m         - Quit Archium\n");
}

void display_help_category(const char *category) {
  if (strcmp(category, "packages") == 0) {
    printf("\n\033[1;33mPackage Operations:\033[0m\n");
    printf("\033[1;32mi\033[0m           - Install packages\n");
    printf("\033[1;32mr\033[0m           - Remove packages\n");
    printf(
        "\033[1;32md\033[0m           - Downgrade packages to cached "
        "versions\n");
    printf(
        "\033[1;32mp\033[0m           - Purge packages (remove with "
        "dependencies)\n");
    printf("\033[1;32ms\033[0m           - Search for packages\n");
    printf(
        "\033[1;32mu\033[0m [package] - Update system or specific package\n");
  } else if (strcmp(category, "system") == 0) {
    printf("\n\033[1;33mSystem Utilities:\033[0m\n");
    printf(
        "\033[1;32mhealth\033[0m      - System health check (disk, integrity, "
        "services)\n");
    printf("\033[1;32mc\033[0m           - Clean package cache\n");
    printf("\033[1;32mcc\033[0m          - Clear build cache\n");
    printf("\033[1;32mo\033[0m           - Clean orphaned packages\n");
    printf("\033[1;32mlo\033[0m          - List orphaned packages\n");
    printf("\033[1;32mcu\033[0m          - Check for package updates\n");
    printf("\033[1;32mba\033[0m          - Backup pacman configuration\n");
  } else if (strcmp(category, "info") == 0) {
    printf("\n\033[1;33mInformation Commands:\033[0m\n");
    printf("\033[1;32ml\033[0m           - List all installed packages\n");
    printf("\033[1;32m?\033[0m           - Show package information\n");
    printf("\033[1;32mdt\033[0m          - Display package dependency tree\n");
    printf("\033[1;32msi\033[0m          - List packages by size\n");
    printf("\033[1;32mre\033[0m          - Show recently installed packages\n");
    printf(
        "\033[1;32mex\033[0m          - List explicitly installed packages\n");
    printf("\033[1;32mow\033[0m          - Find which package owns a file\n");
  } else if (strcmp(category, "config") == 0) {
    printf("\n\033[1;33mConfiguration & Plugins:\033[0m\n");
    printf("\033[1;32mconfig\033[0m      - Configure Archium preferences\n");
    printf("\033[1;32mpl\033[0m          - List loaded plugins\n");
    printf("\033[1;32mpd\033[0m          - View plugin directory\n");
    printf("\033[1;32mpe\033[0m          - Create example plugin\n");
    archium_plugin_display_help();
  } else if (strcmp(category, "plugin") == 0) {
    printf("\n\033[1;33mPlugin Management Commands:\033[0m\n");
    printf("\033[1;32mpl\033[0m          - List loaded plugins\n");
    printf("\033[1;32mpd\033[0m          - View plugin directory\n");
    printf("\033[1;32mpe\033[0m          - Create example plugin\n");
    archium_plugin_display_help();
  } else {
    printf("\n\033[1;31mUnknown category: %s\033[0m\n", category);
    printf(
        "Available categories: \033[1;32mpackages\033[0m, "
        "\033[1;32msystem\033[0m, \033[1;32minfo\033[0m, "
        "\033[1;32mconfig\033[0m, \033[1;32mplugin\033[0m\n");
  }
}

void display_help_quick(void) {
  printf("\n\033[1;33mQuick Reference:\033[0m\n");
  printf(
      "\033[1;32mi\033[0m install  \033[1;32mr\033[0m remove   "
      "\033[1;32md\033[0m downgrade \033[1;32ms\033[0m search   "
      "\033[1;32mu\033[0m update\n");
  printf(
      "\033[1;32ml\033[0m list     \033[1;32mc\033[0m clean    "
      "\033[1;32mo\033[0m orphans  \033[1;32m?\033[0m info\n");
  printf(
      "\033[1;32mq\033[0m quit     \033[1;32mh\033[0m help     "
      "\033[1;32mhealth\033[0m check   \033[1;32mconfig\033[0m       "
      "\033[1;32mpl\033[0m list\n");
}

void display_help_command(const char *command) {
  printf("\n");
  if (strcmp(command, "i") == 0) {
    printf("\033[1;33mInstall Command:\033[0m \033[1;32mi\033[0m\n");
    printf("Install one or more packages from repositories or AUR.\n");
    printf("\033[1;36mExample:\033[0m Install firefox and git\n");
    printf("  Archium $ i\n");
    printf("  Enter package names to install: firefox git\n");
  } else if (strcmp(command, "r") == 0) {
    printf("\033[1;33mRemove Command:\033[0m \033[1;32mr\033[0m\n");
    printf("Remove installed packages while keeping dependencies.\n");
    printf("\033[1;36mExample:\033[0m Remove firefox\n");
    printf("  Archium $ r\n");
    printf("  Enter package names to remove: firefox\n");
  } else if (strcmp(command, "d") == 0) {
    printf("\033[1;33mDowngrade Command:\033[0m \033[1;32md\033[0m\n");
    printf("Downgrade packages to previously cached versions.\n");
    printf("Useful when a new version causes issues.\n");
    printf("\033[1;36mExample:\033[0m Downgrade firefox\n");
    printf("  Archium $ d\n");
    printf("  Enter package names to downgrade: firefox\n");
    printf(
        "\033[1;33mNote:\033[0m Requires cached versions in pacman cache.\n");
  } else if (strcmp(command, "health") == 0) {
    printf("\033[1;33mHealth Check Command:\033[0m \033[1;32mhealth\033[0m\n");
    printf("Perform comprehensive system health diagnostics.\n");
    printf(
        "Checks disk space, package integrity, system services, and more.\n");
    printf("\033[1;36mExample:\033[0m Run system health check\n");
    printf("  Archium $ health\n");
    printf("\033[1;33mChecks performed:\033[0m\n");
    printf("  • Disk space usage analysis\n");
    printf("  • Package file integrity verification\n");
    printf("  • System service status\n");
    printf("  • Memory usage monitoring\n");
    printf("  • Memory usage monitoring\n");
  } else if (strcmp(command, "u") == 0) {
    printf("\033[1;33mUpdate Command:\033[0m \033[1;32mu\033[0m [package]\n");
    printf("Update entire system or specific package.\n");
    printf("\033[1;36mExamples:\033[0m\n");
    printf("  u           - Update entire system\n");
    printf("  u firefox   - Update only firefox\n");
  } else if (strcmp(command, "s") == 0) {
    printf("\033[1;33mSearch Command:\033[0m \033[1;32ms\033[0m\n");
    printf("Search for packages in repositories and AUR.\n");
    printf("\033[1;36mExample:\033[0m Search for text editors\n");
    printf("  Archium $ s\n");
    printf("  Enter package name to search: editor\n");
  } else if (strcmp(command, "tips") == 0) {
    printf("\033[1;33mHelpful Tips:\033[0m\n");
    for (size_t i = 0; i < NUM_TIPS; i++) {
      printf("• %s\n", archium_tips[i]);
    }
  } else if (strcmp(command, "pl") == 0) {
    printf("\033[1;33mPlugin List Command:\033[0m \033[1;32mpl\033[0m\n");
    printf(
        "List all currently loaded plugins with their commands and "
        "descriptions.\n");
    printf("\033[1;36mExample:\033[0m\n");
    printf("  Archium $ pl\n");
  } else if (strcmp(command, "pd") == 0) {
    printf("\033[1;33mPlugin Directory Command:\033[0m \033[1;32mpd\033[0m\n");
    printf(
        "Display the plugin directory path where .so files should be "
        "placed.\n");
    printf("\033[1;36mExample:\033[0m\n");
    printf("  Archium $ pd\n");
  } else if (strcmp(command, "pe") == 0) {
    printf("\033[1;33mPlugin Example Command:\033[0m \033[1;32mpe\033[0m\n");
    printf("Create an example plugin with template code and Makefile.\n");
    printf("\033[1;36mExample:\033[0m\n");
    printf("  Archium $ pe\n");
  } else {
    printf("\033[1;31mNo detailed help available for: %s\033[0m\n", command);
    printf("Try \033[1;32mh quick\033[0m for a command overview.\n");
  }
}
