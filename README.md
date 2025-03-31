# 🌟 Archium - Fast & Easy Package Management for Arch Linux

## 🚀 Current Version

**Archium v1.5.3**

Archium is a command-line tool for managing packages on Arch Linux. It provides a simple and intuitive interface for common package management tasks using `yay`, `paru`, or `pacman`.

---

## ✨ Features

- 🔄 **Update the system** or specific packages
- 📦 **Install packages**
- ❌ **Remove packages**
- 🗑️ **Purge packages**
- 🧹 **Clean cache**
- 🧩 **Clean orphaned packages**
- 🔍 **Search for packages**
- 📋 **List installed packages**
- ❓ **Display package information**
- 🌳 **Display package dependency tree**
- 🔔 **Check for updates**

---

## 🛠️ Installation

### 1. Clone the Repository
```bash
git clone https://github.com/q4ow/archium.git
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

---

## 📖 Usage

When you run `archium`, you will see the following prompt:

```plaintext
Welcome to Archium, type "h" for help
Archium $
```

### Interactive Shell Commands
| Command | Description                              |
|---------|------------------------------------------|
| `u`     | Update the system or a specific package  |
| `i`     | Install packages                         |
| `r`     | Remove packages                          |
| `p`     | Purge packages                           |
| `c`     | Clean cache                              |
| `cc`    | Clear package build cache               |
| `o`     | Clean orphaned packages                 |
| `lo`    | List orphaned packages                  |
| `s`     | Search for packages                     |
| `l`     | List installed packages                 |
| `?`     | Display package information             |
| `dt`    | Display package dependency tree         |
| `cu`    | Check for updates                       |
| `h`     | Display help                            |
| `q`     | Quit the application                    |

### Example
To install a package, type `i` and follow the prompt:
```plaintext
$ i
Enter package name to install:
```

---

### Command-Line Arguments

| Argument            | Description                                      |
|----------------------|--------------------------------------------------|
| `--exec <command>`   | Execute a specific command directly              |
| `--version`, `-v`    | Display version information                      |
| `--verbose`, `-V`    | Enable verbose logging                           |
| `--help`, `-h`       | Display help for command-line arguments          |
| `--self-update`      | Update Archium to the latest version            |

### Example
To update the system directly from the command line:
```bash
archium --exec u
```

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

---

## 📦 Dependencies

- **`gcc`** - GNU Compiler Collection
- **`yay`** or **`paru`** - AUR helpers for Arch Linux
- **`git`** - For installing `yay` if it is not already installed
- **`readline`** - A library for command-line input (likely preinstalled)
- **`ncurses`** - A library for text-based user interfaces (likely preinstalled)

---

## 📝 Notes

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

---

## 📜 License

This program is licensed under the **GNU General Public License v3.0**.  
See the [LICENSE](./LICENSE) file for details.

---

## ❤️ Contributing

Contributions are welcome! Feel free to fork the repository and submit a pull request.

---

## 📧 Contact

For any questions or issues, please contact [Keiran](mailto:me@keiran.cc).

---

> **Disclaimer**: Archium is not affiliated with Arch Linux or its official package managers.
