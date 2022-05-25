## Advanced POS CLI Usage
Based on the UNIX philosophy, POS CLI is designed to be combined with linux commands to provide a number of advanced functionalities. This document describes an advnaced guide to POS CLI with some examples.

### Command Auto Completion
Based on [Cobra](https://github.com/spf13/cobra), POS CLI provides command autocompletion for bash and zsh. 

Executing the following command will display how to enable the command autocompletion:
```bash
/poseidonos/ibofos/bin$ ./poseidonos-cli completion --help

Completion command generates an auto-completion script for bash, zsh, fish, or powershell.

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
  # you will need to enable it.  You can execute the following once:

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

Usage:
  poseidonos-cli completion [bash|zsh|fish|powershell]

```

Follow the instructions according to your shell. After restarting the shell, you will be able to TAB to complete the command automatically.

### Combining POS CLI with Linux Commands
You can combine POS CLI with the popular linux commands such as grep and awk. Here, we describe some examples. 

#### Conditional Output Display
The device list command will display all the devices in the system. 
```bash
/poseidonos/bin$ ./poseidonos-cli device list
| Name       | SerialNumber(SN) | Address      | Class  | MN              | NUMA | Size          |
| ---------- | ---------------- | ------------ | ------ | --------------- | ---- | ------------- |
| unvme-ns-0 | 016              | 0000:4e:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-1 | 017              | 0000:52:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-2 | 018              | 0000:69:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-3 | 019              | 0000:6c:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-4 | 020              | 0000:ce:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
| unvme-ns-5 | 021              | 0000:d1:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
| unvme-ns-6 | 022              | 0000:e7:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
| unvme-ns-7 | 023              | 0000:ea:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
```
Using grep, you can display the information of specific devices, arrays, or volumes as in the following examples:

```bash
# Displaying the information of "unvme-ns-4" only.
/poseiondonos/bin$ ./poseidonos-cli device list | grep unvme-ns-4
unvme-ns-4     |020                  |0000:ce:00.0   |SYSTEM        |SAMSUNG SSD -Q-                          |1      |3840755982336

# Displaying the information of all the devices except for "unvme-ns-4".
/poseiondonos/bin$ ./poseidonos-cli device list | grep -v unvme-ns-4
| Name       | SerialNumber(SN) | Address      | Class  | MN              | NUMA | Size          |
| ---------- | ---------------- | ------------ | ------ | --------------- | ---- | ------------- |
| unvme-ns-0 | 016              | 0000:4e:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-1 | 017              | 0000:52:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-2 | 018              | 0000:69:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-3 | 019              | 0000:6c:00.0 | SYSTEM | SAMSUNG SSD -Q- | 0    | 3840755982336 |
| unvme-ns-5 | 021              | 0000:d1:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
| unvme-ns-6 | 022              | 0000:e7:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
| unvme-ns-7 | 023              | 0000:ea:00.0 | SYSTEM | SAMSUNG SSD -Q- | 1    | 3840755982336 |
```

Using awk, you can display the conditional information of specific devices, arrays, or volumes as in the following examples:

```bash
# Displaying the name, the serial number, and the size of the devices whose serial number is greater than 19.
/poseidonos/bin$ ./poseidonos-cli device list | awk -F "|" '$2 > 19 {print $1 $2 $7 }'
Name           SerialNumber(SN)     Size
unvme-ns-4     020                  3840755982336
unvme-ns-5     021                  3840755982336
unvme-ns-6     022                  3840755982336
unvme-ns-7     023                  3840755982336
```
- Note: you need to set the field separator of awk to '|'. POS CLI uses '|' as the default field separator (because there can be some whitespace in the data). You can change the field separator of POS CLI using the --fs option.

Try building your own custom command lines and scripts to fully utilize the potential of POS!
