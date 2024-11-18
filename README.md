# Archium - Fast & Easy Package Management for Arch Linux

Archium is a command-line tool for managing packages on Arch Linux, providing a simple interface for common package management tasks using `yay` or `paru`.

## Features

- Update the system
- Install packages
- Remove packages
- Purge packages
- Clean cache
- Clean orphaned packages
- Search for packages

## Installation

1. **Clone the repository:**

    ```sh
    git clone https://github.com/KeiranScript/archium.git
    cd archium
    ```

2. **Compile the program:**

    ```sh
    make
    ```

3. **Install the binary to `/usr/bin`** (requires root permissions):

    ```sh
    sudo make install
    ```

## Usage

When you run `archium`, you will see the following prompt:
```
Welcome to Archium, type "h" for help
Archium $
```

You can then use the following commands:

- `u` [package] - Update the system or specific package
- `i`         - Install packages
- `r`         - Remove packages
- `p`         - Purge packages
- `c`         - Clean cache
- `cc`        - clean build cache
- `o`         - Clean orphaned packages
- `lo`        - List orphaned packages
- `s`         - Search for packages
- `l`         - List installed packages
- `?`         - Display package information
- `dt`        - Display package dependency tree
- `cu`        - Check for updates
- `h`         - Display help
- `q`         - Quit the application

### Example

To install a package, type `i` and follow the prompt:

 ```
 $ i
 Enter package name to install:
 ```

## Command-Line Arguments

- `--exec <command>` - Execute a specific command without entering the interactive prompt. The `<command>` can be one of the following:


- `u` [package] - Update the system or specific package
- `i`         - Install packages
- `r`         - Remove packages
- `p`         - Purge packages
- `c`         - Clean cache
- `cc`        - clean build cache
- `o`         - Clean orphaned packages
- `lo`        - List orphaned packages
- `s`         - Search for packages
- `l`         - List installed packages
- `?`         - Display package information
- `dt`        - Display package dependency tree
- `cu`        - Check for updates
- `h`         - Display help
- `q`         - Quit the application

   Example:

   ```sh
   archium --exec u
   ```

   This command will update the system directly from the command line. If you run `archium --exec` without specifying a command, you will be prompted to enter a command interactively.

- `--version` - Display the version information:

    ```sh
    archium --version
    ```

## Dependencies

- `gcc` - GNU Compiler Collection
- `yay` or `paru` - AUR helpers for Arch Linux
- `git` - For installing `yay` if it is not already installed
 
- `readline` - A library for command-line input, probably preinstalled
- `ncurses` - A library for text-based user interfaces, probably preinstalled

## Notes
> [!NOTE]
    Archium uses yay by default, but if you only have paru installed it'll use it. If you have both and you want to use `paru` instead of `yay` create a file in `$HOME` called `.archium-use-paru`, and it will install `paru` instead of `yay`

```sh
touch $HOME/.archium-use-paru
```

> [!NOTE]
    If Archium fails to upgrade to a newer version, try cleaning cache using your prefered AUR helper of choice and installing it again, example given below

```sh
 yay -Scc --noconfirm && yay -S archium --noconfirm

 paru -Scc --noconfirm && paru -S archium --noconfirm
```

## License

This program is licensed under the GNU General Public License. See the [LICENSE](https://github.com/KeiranScript/archium/blob/main/LICENSE) file for details.