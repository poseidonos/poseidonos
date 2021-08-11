package develcmds

import (
	"github.com/spf13/cobra"
)

var DevelCmd = &cobra.Command{
	Use:   "devel",
	Short: "Commands for PoseidonOS Developers.",
	Long: `Commands for PoseidonOS Developers.

Syntax: 
  poseidonos-cli devel [resetmbr]

	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	DevelCmd.AddCommand(ResetMBRCmd)
}
