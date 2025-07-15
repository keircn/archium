#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMPLETIONS_DIR="$SCRIPT_DIR/completions"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

detect_shell() {
    if [[ -n "$ZSH_VERSION" ]]; then
        echo "zsh"
    elif [[ -n "$BASH_VERSION" ]]; then
        echo "bash"
    elif [[ -n "$FISH_VERSION" ]]; then
        echo "fish"
    else
        case "$(basename "$SHELL")" in
            zsh) echo "zsh" ;;
            bash) echo "bash" ;;
            fish) echo "fish" ;;
            *) echo "unknown" ;;
        esac
    fi
}

install_bash_completion() {
    local system_dirs=("/usr/share/bash-completion/completions" "/etc/bash_completion.d")
    local user_dir="$HOME/.bash_completion.d"
    local installed=false

    for dir in "${system_dirs[@]}"; do
        if [[ -d "$dir" && -w "$dir" ]]; then
            print_status "Installing bash completion to $dir"
            cp "$COMPLETIONS_DIR/archium.bash" "$dir/archium"
            installed=true
            break
        elif [[ -d "$dir" ]]; then
            print_status "Installing bash completion to $dir (requires sudo)"
            if sudo cp "$COMPLETIONS_DIR/archium.bash" "$dir/archium" 2>/dev/null; then
                installed=true
                break
            fi
        fi
    done

    if [[ "$installed" == false ]]; then
        print_status "Installing bash completion to user directory: $user_dir"
        mkdir -p "$user_dir"
        cp "$COMPLETIONS_DIR/archium.bash" "$user_dir/archium"
        
        local bashrc="$HOME/.bashrc"
        if [[ -f "$bashrc" ]] && ! grep -q "\.bash_completion\.d" "$bashrc"; then
            echo "" >> "$bashrc"
            echo "# Load custom bash completions" >> "$bashrc"
            echo "[[ -d ~/.bash_completion.d ]] && for f in ~/.bash_completion.d/*; do [[ -f \$f ]] && source \"\$f\"; done" >> "$bashrc"
            print_status "Added completion loader to ~/.bashrc"
        fi
        installed=true
    fi

    if [[ "$installed" == true ]]; then
        print_success "Bash completion installed successfully"
        print_status "Restart your shell or run 'source ~/.bashrc' to enable completions"
    else
        print_error "Failed to install bash completion"
        return 1
    fi
}

install_zsh_completion() {
    local system_dirs=("/usr/share/zsh/site-functions" "/usr/local/share/zsh/site-functions")
    local user_dir="$HOME/.zsh/completions"
    local installed=false

    for dir in "${system_dirs[@]}"; do
        if [[ -d "$dir" && -w "$dir" ]]; then
            print_status "Installing zsh completion to $dir"
            cp "$COMPLETIONS_DIR/archium.zsh" "$dir/_archium"
            installed=true
            break
        elif [[ -d "$dir" ]]; then
            print_status "Installing zsh completion to $dir (requires sudo)"
            if sudo cp "$COMPLETIONS_DIR/archium.zsh" "$dir/_archium" 2>/dev/null; then
                installed=true
                break
            fi
        fi
    done

    if [[ "$installed" == false ]]; then
        print_status "Installing zsh completion to user directory: $user_dir"
        mkdir -p "$user_dir"
        cp "$COMPLETIONS_DIR/archium.zsh" "$user_dir/_archium"
        
        local zshrc="$HOME/.zshrc"
        if [[ -f "$zshrc" ]] && ! grep -q "$user_dir" "$zshrc"; then
            echo "" >> "$zshrc"
            echo "# Add custom completions to fpath" >> "$zshrc"
            echo "fpath=(~/.zsh/completions \$fpath)" >> "$zshrc"
            echo "autoload -U compinit && compinit" >> "$zshrc"
            print_status "Added completion directory to ~/.zshrc"
        fi
        installed=true
    fi

    if [[ "$installed" == true ]]; then
        print_success "Zsh completion installed successfully"
        print_status "Restart your shell or run 'source ~/.zshrc' to enable completions"
    else
        print_error "Failed to install zsh completion"
        return 1
    fi
}

install_fish_completion() {
    local system_dirs=("/usr/share/fish/completions" "/usr/local/share/fish/completions")
    local user_dir="$HOME/.config/fish/completions"
    local installed=false

    for dir in "${system_dirs[@]}"; do
        if [[ -d "$dir" && -w "$dir" ]]; then
            print_status "Installing fish completion to $dir"
            cp "$COMPLETIONS_DIR/archium.fish" "$dir/archium.fish"
            installed=true
            break
        elif [[ -d "$dir" ]]; then
            print_status "Installing fish completion to $dir (requires sudo)"
            if sudo cp "$COMPLETIONS_DIR/archium.fish" "$dir/archium.fish" 2>/dev/null; then
                installed=true
                break
            fi
        fi
    done

    if [[ "$installed" == false ]]; then
        print_status "Installing fish completion to user directory: $user_dir"
        mkdir -p "$user_dir"
        cp "$COMPLETIONS_DIR/archium.fish" "$user_dir/archium.fish"
        installed=true
    fi

    if [[ "$installed" == true ]]; then
        print_success "Fish completion installed successfully"
        print_status "Fish completions are loaded automatically"
    else
        print_error "Failed to install fish completion"
        return 1
    fi
}

show_usage() {
    echo "Usage: $0 [OPTIONS] [SHELL]"
    echo ""
    echo "Install shell completions for archium"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -a, --all      Install for all available shells"
    echo "  -l, --list     List available completion scripts"
    echo ""
    echo "Shells:"
    echo "  bash           Install bash completion"
    echo "  zsh            Install zsh completion"  
    echo "  fish           Install fish completion"
    echo "  auto           Auto-detect current shell (default)"
    echo ""
    echo "Examples:"
    echo "  $0             Auto-detect and install for current shell"
    echo "  $0 zsh         Install zsh completion"
    echo "  $0 --all       Install for all shells"
}

list_completions() {
    echo "Available completion scripts:"
    for file in "$COMPLETIONS_DIR"/*; do
        if [[ -f "$file" ]]; then
            local basename=$(basename "$file")
            local shell="${basename##*.}"
            echo "  $shell: $file"
        fi
    done
}

main() {
    if [[ ! -d "$COMPLETIONS_DIR" ]]; then
        print_error "Completions directory not found: $COMPLETIONS_DIR"
        exit 1
    fi

    local target_shell="auto"
    local install_all=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -a|--all)
                install_all=true
                shift
                ;;
            -l|--list)
                list_completions
                exit 0
                ;;
            bash|zsh|fish|auto)
                target_shell="$1"
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done

    print_status "Archium Shell Integration Installer"
    echo ""

    if [[ "$install_all" == true ]]; then
        print_status "Installing completions for all shells..."
        local success=0
        
        if [[ -f "$COMPLETIONS_DIR/archium.bash" ]]; then
            install_bash_completion && ((success++))
        fi
        
        if [[ -f "$COMPLETIONS_DIR/archium.zsh" ]]; then
            install_zsh_completion && ((success++))
        fi
        
        if [[ -f "$COMPLETIONS_DIR/archium.fish" ]]; then
            install_fish_completion && ((success++))
        fi
        
        echo ""
        print_success "Installed completions for $success shell(s)"
        
    else
        if [[ "$target_shell" == "auto" ]]; then
            target_shell=$(detect_shell)
            print_status "Detected shell: $target_shell"
        fi

        case "$target_shell" in
            bash)
                if [[ -f "$COMPLETIONS_DIR/archium.bash" ]]; then
                    install_bash_completion
                else
                    print_error "Bash completion script not found"
                    exit 1
                fi
                ;;
            zsh)
                if [[ -f "$COMPLETIONS_DIR/archium.zsh" ]]; then
                    install_zsh_completion
                else
                    print_error "Zsh completion script not found"
                    exit 1
                fi
                ;;
            fish)
                if [[ -f "$COMPLETIONS_DIR/archium.fish" ]]; then
                    install_fish_completion
                else
                    print_error "Fish completion script not found"
                    exit 1
                fi
                ;;
            unknown)
                print_error "Could not detect shell. Please specify one manually."
                echo ""
                show_usage
                exit 1
                ;;
            *)
                print_error "Unsupported shell: $target_shell"
                exit 1
                ;;
        esac
    fi
}

main "$@"
