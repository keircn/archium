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
║      Welcome to Archium v1.6.2         ║
║      Type "h" for help                 ║
╚════════════════════════════════════════╝
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

## Notes

- Archium uses `yay` by default. If you only have `paru` installed, it will use `paru`.  
  If you have both and want to use `paru` instead of `yay`, create a file in `$HOME` called `.archium-use-paru`:

  ```bash
  touch $HOME/.archium-use-paru
  ```

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
