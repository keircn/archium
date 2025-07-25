complete -c archium -f
complete -c archium -l help -s h -d 'Show help message'
complete -c archium -l version -s v -d 'Show version information'
complete -c archium -l verbose -s V -d 'Enable verbose logging'
complete -c archium -l exec -d 'Execute command directly' -x
complete -c archium -l self-update -d 'Update Archium to latest version'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a 'h help' -d 'Show help'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a u -d 'Update system'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a i -d 'Install packages'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a r -d 'Remove packages'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a p -d 'Purge packages'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a c -d 'Clean cache'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a cc -d 'Clear build cache'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a o -d 'Clean orphans'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a lo -d 'List orphans'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a s -d 'Search packages'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a l -d 'List installed packages'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a '?' -d 'Show package info'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a cu -d 'Check updates'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a dt -d 'Display dependency tree'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a si -d 'List packages by size'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a re -d 'List recent installs'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a ex -d 'List explicit installs'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a ow -d 'Find package owner'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a ba -d 'Backup pacman config'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a config -d 'Configure preferences'
complete -c archium -n '__fish_seen_subcommand_from --exec' -a 'plugin plugins' -d 'Manage plugins'
