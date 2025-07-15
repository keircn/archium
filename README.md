# Archium - Fast & Easy Package Management for Arch Linux

Archium is a command-line tool for managing packages on Arch Linux. It provides a simple and intuitive interface for common package management tasks using `yay`, `paru`, or `pacman`. Archium is a faithful fork of [Archie](https://github.com/TuxForge/archie) by [Gurov](https://github.com/Gur0v).

## Dependencies

- **`gcc`** - It's written in C, what do you expect?
- **`yay`** or **`paru`** - AUR helpers for Arch Linux [OPTIONAL]
- **`git`** - For installing `yay` if it is not already installed [OPTIONAL]
- **`readline`** - A library for command-line input (likely preinstalled)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/keircn/archium.git
cd archium
```

### 2. Compile the Program

```bash
make
```

### 3. Install the Binary (Requires Root Permissions)

```bash
sudo make install
```

## Usage

When you run `archium`, you will see the following prompt:

```plaintext
╔════════════════════════════════════════╗
║      Welcome to Archium v1.8.1         ║
║      Type "h" for help                 ║
╚════════════════════════════════════════╝
Archium $ u
Upgrading system...
```

### Command-Line Arguments

| Argument           | Description                             |
| ------------------ | --------------------------------------- |
| `--exec <command>` | Execute a specific command directly     |
| `--version`, `-v`  | Display version information             |
| `--verbose`, `-V`  | Enable verbose logging                  |
| `--help`, `-h`     | Display help for command-line arguments |
| `--self-update`    | Update Archium to the latest version    |

To update Archium itself (only for manual installations):

```bash
archium --self-update
```

**Note**: The `--self-update` option only works if you installed Archium manually.
If you installed Archium from the AUR, please use your AUR helper to update:

```bash
yay -Syu archium
# or
paru -Syu archium
```

## Configuration

Archium uses a centralized configuration system located at `$HOME/.config/archium/`:

- **Configuration Directory**: `$HOME/.config/archium/`
- **Log File**: `$HOME/.config/archium/archium.log` (when verbose mode is enabled)
- **Preferences**: `$HOME/.config/archium/preferences`
- **Cache Directory**: `$HOME/.config/archium/cache/`

### Setting Package Manager Preference

You can set your preferred package manager using the built-in configuration command:

```bash
archium

Archium $ config
Archium Configuration
Available preferences:
1. Package manager preference (yay/paru)
2. View current configuration directory
3. View log file location
Enter your choice (1-3): 1
Current preference: yay (default)
Set package manager preference (yay/paru): paru
Package manager preference set to: paru
Note: Restart Archium for changes to take effect.
```

Alternatively, for quick command-line usage:

```bash
archium --exec config
```

### Legacy Migration

If you have an existing `.archium-use-paru` file in your home directory, Archium will automatically migrate it to the new configuration system and remove the old file.

## Plugin System

Archium features an extensible plugin system that allows you to add custom commands and functionality through shared libraries (.so files).

### Plugin Directory

Plugins are stored in: `$HOME/.config/archium/plugins/`

### Managing Plugins

Use the built-in plugin management system:

```bash
# Run Archium and use the plugin command
archium
# Then type: plugin
```

Available plugin management options:

1. **List loaded plugins** - See all currently loaded plugins
2. **View plugin directory** - Get the path to the plugin directory
3. **Create example plugin** - Generate example plugin source code and Makefile

### Creating Plugins

Plugins are C shared libraries that implement a specific API. To create a plugin:

1. **Generate example**: Use `plugin` command → option 3 to create `example.c` and `Makefile`
2. **Customize**: Modify the generated code to implement your functionality
3. **Build**: Run `make` in the plugin directory to create the `.so` file
4. **Load**: Restart Archium to automatically load the new plugin

### Plugin API

Each plugin must implement these functions:

```c
char *archium_plugin_get_name(void);           // Plugin display name
char *archium_plugin_get_command(void);        // Command name
char *archium_plugin_get_description(void);    // Command description
ArchiumError archium_plugin_execute(const char *args, const char *package_manager);
void archium_plugin_cleanup(void);             // Optional cleanup
```

### Plugin Example

```c
char *archium_plugin_get_command(void) {
  return "hello";
}

ArchiumError archium_plugin_execute(const char *args, const char *package_manager) {
  printf("Hello from plugin! Package manager: %s\n", package_manager);
  return ARCHIUM_SUCCESS;
}
```

Build with: `gcc -fPIC -shared -o hello.so hello.c`

## Notes

- Archium uses `yay` by default. If you only have `paru` installed, it will use `paru`.
- Use the `config` command within Archium to manage preferences and view configuration paths.
- Enable verbose logging with `--verbose` or `-V` to create detailed logs at `$HOME/.config/archium/archium.log`.

- If Archium fails to upgrade to a newer version, try cleaning the cache using your preferred AUR helper and reinstalling:
  ```bash
  yay -Scc --noconfirm && yay -S archium --noconfirm
  paru -Scc --noconfirm && paru -S archium --noconfirm
  ```

## License

This program is licensed under the **GNU General Public License v3.0**.  
See the [LICENSE](./LICENSE) file for details.

## Contact

For any questions or issues, please contact [Keiran](mailto:keiran0@proton.me).

> **Disclaimer**: Archium is not affiliated with Arch Linux or its official package managers.
