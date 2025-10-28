#!/bin/bash

_archium() {
    local cur prev words cword
    _init_completion || return

    local flags="--help -h --version -v --verbose -V --exec --self-update"
    local exec_commands="h help u i r p c cc o lo s l ? cu dt si re ex ow ba config plugin plugins pl pd pe"

    case $COMP_CWORD in
        1)
            COMPREPLY=($(compgen -W "$flags" -- "$cur"))
            ;;
        2)
            case "$prev" in
                --exec)
                    COMPREPLY=($(compgen -W "$exec_commands" -- "$cur"))
                    ;;
            esac
            ;;
    esac

    return 0
}

complete -F _archium archium
