#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

int check_package_manager(void);
int check_command(const char *command);
int check_git(void);
void prompt_install_yay(void);
void install_yay(void);
void update_system(const char *package_manager, const char *package);
void install_package(const char *package_manager, const char *packages);
void remove_package(const char *package_manager, const char *packages);
void purge_package(const char *package_manager, const char *packages);
void clean_cache(const char *package_manager);
void clean_orphans(const char *package_manager);
void search_package(const char *package_manager, const char *package);
void list_installed_packages(void);
void show_package_info(const char *package_manager, const char *package);
void check_package_updates(void);
void display_dependency_tree(const char *package_manager, const char *package);
void clear_build_cache(void);
void list_orphans(void);
void list_packages_by_size(void);
void list_recent_installs(void);
void list_explicit_installs(void);
void find_package_owner(const char *file);
void backup_pacman_config(void);
void configure_preferences(void);
char **get_pacman_commands(void);
char *get_package_manager_version(const char *package_manager);

#endif