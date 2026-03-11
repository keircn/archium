# Archium

Fast and configurable package management for Arch Linux.

Archium is a C-based CLI wrapper around `yay`, `paru`, and `pacman` with short
commands, readline support, and plugin hooks.

```text
u           # upgrade everything
i firefox   # install firefox
r firefox   # remove firefox
s neovim    # search for neovim
```

## Installation

### Dependencies

- `gcc`
- `readline`
- One of: `yay`, `paru`, or `pacman`
- `git` (only needed if you choose auto-install flow for `yay`)

### 1. Clone the Repository

```bash
git clone https://github.com/keircn/archium
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

### Command-Line Arguments

| Argument                 | Description                             |
| ------------------------ | --------------------------------------- |
| `--exec <command>`       | Execute a specific command directly     |
| `--version`, `-v`        | Display version information             |
| `--verbose`, `-V`        | Enable verbose logging                  |
| `--help`, `-h`           | Display help for command-line arguments |
| `--self-update`          | Update Archium to the latest version    |
| `--json`                 | Emit machine-readable output            |
| `--batch`                | Disable interactive prompts             |
| `--custom-output`, `-c`  | Use Archium custom output mode          |

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

Archium stores configuration in `$HOME/.config/archium/`:

- `preferences` (key/value settings)
- `archium.log` (when verbose mode is enabled)
- `cache/` (runtime cache)
- `plugins/` (plugin `.so` files)

### Preferences File

Path: `$HOME/.config/archium/preferences`

Supported keys:

```ini
package_manager=yay
json_output=0
batch_mode=0
use_native_output=1
show_welcome=1
show_tips=1
cache_ttl_seconds=3600
```

Validation rules:

- `package_manager`: `yay` or `paru`
- `json_output`, `batch_mode`, `use_native_output`, `show_welcome`, `show_tips`:
  `0`, `1`, `false`, or `true`
- `cache_ttl_seconds`: integer from `60` to `86400`

Invalid lines are ignored at read-time, and invalid writes/imports are rejected.

### Environment Variable Overrides

Environment variables override file values for the current process:

- `ARCHIUM_PACKAGE_MANAGER`
- `ARCHIUM_JSON_OUTPUT`
- `ARCHIUM_BATCH_MODE`
- `ARCHIUM_NATIVE_OUTPUT`
- `ARCHIUM_SHOW_WELCOME`
- `ARCHIUM_SHOW_TIPS`
- `ARCHIUM_CACHE_TTL_SECONDS`

Example:

```bash
ARCHIUM_SHOW_TIPS=0 ARCHIUM_CACHE_TTL_SECONDS=120 archium
```

### Interactive Config Menu

Run:

```bash
archium --exec config
```

The config menu supports:

- toggling UI/output defaults
- setting package cache TTL
- viewing effective configuration
- exporting/importing preferences
- backup/restore for preferences

## Plugin System

Archium features an extensible plugin system that allows you to add custom commands and functionality through shared libraries (.so files).

### Plugin Directory

Plugins are stored in: `$HOME/.config/archium/plugins/`

### Managing Plugins

Use the built-in plugin management commands:

Available plugin management commands:

- **`pl`** - List all currently loaded plugins with their commands and descriptions
- **`pd`** - Display the plugin directory path where .so files should be placed
- **`pe`** - Create an example plugin with template code and Makefile

### Creating Plugins

Plugins are C shared libraries that implement a specific API. To create a plugin:

1. **Generate example**: Use the `pe` command to create `example.c` and `Makefile`
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

Optional hooks (API v2):

```c
int archium_plugin_get_api_version(void);      // Return ARCHIUM_PLUGIN_API_VERSION
void archium_plugin_init(const ArchiumPluginContext *ctx);
ArchiumError archium_plugin_before_command(const ArchiumPluginContext *ctx);
void archium_plugin_after_command(const ArchiumPluginContext *ctx, ArchiumError result);
void archium_plugin_on_exit(const ArchiumPluginContext *ctx);
```

The `ArchiumPluginContext` provides `command`, `args`, `package_manager`,
config paths, and logging/command helper callbacks.

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

### Examples

Example plugins are available in `examples/`, including a basic hello plugin and
a v2 plugin using hooks. See `examples/README.md` for build steps.

## Notes

- Archium picks package managers in this order: configured preference (if installed), then available fallback (`yay` -> `paru` -> `pacman`).
- Use `config` (or edit `preferences`) to customize behavior.
- Enable verbose logging with `--verbose` or `-V`.

- If Archium fails to upgrade to a newer version, try cleaning the cache using your preferred AUR helper and reinstalling:

  ```bash
  yay -Scc --noconfirm && yay -S archium --noconfirm
  paru -Scc --noconfirm && paru -S archium --noconfirm
  ```

## License

This program is licensed under the **BSD 3-Clause License**.
See the [LICENSE](./LICENSE) file for details.

## Contact

For any questions or issues, please contact me at [keircn@proton.me](mailto:keircn@proton.me) or via [Discord](https://keircn.com/discord).

> **Disclaimer**: Archium is not affiliated with Arch Linux or its official package managers.
