package develcmds

import (
	"github.com/spf13/cobra"
)

var DevelCmd = &cobra.Command{
	Use:   "devel",
	Short: "Commands for PoseidonOS developers.",
	Long: `
Commands for PoseidonOS Developers. This command category will affect
the system seriously. Therefore, this command category must be carefully used
by developers. 

Syntax: 
  poseidonos-cli devel [resetmbr|reset-event-wrr|stop-rebuilding|update--event-wrr]

	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	DevelCmd.AddCommand(ResetMBRCmd)
	DevelCmd.AddCommand(StopRebuildingCmd)
	DevelCmd.AddCommand(UpdateEventWrrCmd)
	DevelCmd.AddCommand(ResetEventWrrCmd)
}
