## poseidonos-cli completion

Generate completion script

### Synopsis


Completion command generates an auto completion script for bash, zsh, fish, or powershell.

To load completions:

Bash:

  $ source <(poseidonos-cli completion bash)

  # To enable auto completions for every session, execute the following:
  # Linux:
  $ sudo -s
  $ poseidonos-cli completion bash > /etc/bash_completion.d/poseidonos-cli
  $ exit
  $ exec bash (restart session).

  # macOS:
  $ poseidonos-cli completion bash > /usr/local/etc/bash_completion.d/poseidonos-cli

Zsh:

  # If shell completion is not already enabled in your environment,
  # you will need to enable it. You can execute the following once:

  $ echo "autoload -U compinit; compinit" >> ~/.zshrc

  # To load completions for each session, execute once:
  $ poseidonos-cli completion zsh > "${fpath[1]}/_poseidonos-cli"
  
  If the command above does not work, try
  $ poseidonos-cli completion zsh > "/usr/local/share/zsh/site-functions/_poseidonos-cli"

  # You will need to start a new shell for this setup to take effect.

Fish:

  $ poseidonos-cli completion fish | source

  # To load completions for each session, execute once:
  $ poseidonos-cli completion fish > ~/.config/fish/completions/poseidonos-cli.fish

PowerShell:

  PS> poseidonos-cli completion powershell | Out-String | Invoke-Expression

  # To load completions for every new session, run:
  PS> poseidonos-cli completion powershell > poseidonos-cli.ps1
  # Add source /yourpath/poseidonos-cli.ps1 file from your PowerShell profile.


```
poseidonos-cli completion [bash|zsh|fish|powershell]
```

### Options

```
  -h, --help   help for completion
```

### Options inherited from parent commands

```
      --debug            Print response for debug.
      --fs string        Field separator for the output. (default "|")
      --ip string        Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req         Print request in JSON form.
      --json-res         Print response in JSON form.
      --node string      Name of the node to send this command. When both --ip and this flag are specified, this flag is applied only.
      --port string      Set the port number to PoseidonOS for this command. (default "18716")
      --timeout uint32   Timeout for this command in seconds. (Note: array unmount command has 30 minutes timeout.) (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS

