#compdef archium

_archium() {
    local -a flags
    flags=(
        '--help:Show help message'
        '-h:Show help message'
        '--version:Show version information'
        '-v:Show version information'
        '--verbose:Enable verbose logging'
        '-V:Enable verbose logging'
        '--exec:Execute command directly'
        '--self-update:Update Archium to latest version'
    )

    local -a exec_commands
    exec_commands=(
        'h:Show help'
        'help:Show help'
        'u:Update system'
        'i:Install packages'
        'r:Remove packages'
        'p:Purge packages'
        'c:Clean cache'
        'cc:Clear build cache'
        'o:Clean orphans'
        'lo:List orphans'
        's:Search packages'
        'l:List installed packages'
        '?:Show package info'
        'cu:Check updates'
        'dt:Display dependency tree'
        'si:List packages by size'
        're:List recent installs'
        'ex:List explicit installs'
        'ow:Find package owner'
        'ba:Backup pacman config'
        'config:Configure preferences'
        'plugin:Manage plugins'
        'plugins:Manage plugins'
        'pl:List loaded plugins'
        'pd:View plugin directory'
        'pe:Create example plugin'
    )

    if (( CURRENT == 1 )); then
        _describe 'flags' flags
    elif (( CURRENT == 2 )); then
        case "$words[1]" in
            --exec)
                _describe 'commands' exec_commands
                ;;
        esac
    fi
}

compdef _archium archium

# vim: ft=zsh
