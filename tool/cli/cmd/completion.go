package cmd

import (
	"os"

	"github.com/spf13/cobra"
)

var completionCmd = &cobra.Command{
	Use:   "completion [bash|zsh]",
	Short: "Generate completion script",
	Long: `To load completions:

Bash:

  $ source <(poseidonos-cli completion bash)

  # To enable auto completions for every session, execute the following:
  # Linux:
  $ sudo -s
  $ poseidonos-cli completion bash > /etc/bash_completion.d/poseidonos-cli
  # Restart session.
  # macOS:
  $ poseidonos-cli completion bash > /usr/local/etc/bash_completion.d/poseidonos-cli

Zsh:

  # If shell completion is not already enabled in your environment,
  # you will need to enable it.  You can execute the following once:

  $ echo "autoload -U compinit; compinit" >> ~/.zshrc

  # To load completions for each session, execute once:
  $ poseidonos-cli completion zsh > "${fpath[1]}/_poseidonos-cli"

  # You will need to start a new shell for this setup to take effect.
`,
	DisableFlagsInUseLine: true,
	ValidArgs:             []string{"bash", "zsh"},
	Args:                  cobra.ExactValidArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		switch args[0] {
		case "bash":
			cmd.Root().GenBashCompletion(os.Stdout)
		case "zsh":
			cmd.Root().GenZshCompletion(os.Stdout)
		}
	},
}
