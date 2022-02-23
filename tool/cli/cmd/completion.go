package cmd

import (
	"os"

	"github.com/spf13/cobra"
)

var completionCmd = &cobra.Command{
	Use:   "completion [bash|zsh|fish|powershell]",
	Short: "Generate completion script",
	Long: `
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
`,
	DisableFlagsInUseLine: true,
	ValidArgs:             []string{"bash", "zsh", "fish", "powershell"},
	Args:                  cobra.ExactValidArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		switch args[0] {
		case "bash":
			cmd.Root().GenBashCompletion(os.Stdout)
		case "zsh":
			cmd.Root().GenZshCompletion(os.Stdout)
		case "fish":
			cmd.Root().GenFishCompletion(os.Stdout, true)
		case "powershell":
			cmd.Root().GenPowerShellCompletionWithDesc(os.Stdout)
		}
	},
}
