package systemcmds

import (
	"github.com/spf13/cobra"
)

var SystemCmd = &cobra.Command{
	Use:   "system",
	Short: "System commands for PoseidonOS.",
	Long: `System commands for PoseidonOS.

Syntax: 
  poseidonos-cli system [start|stop|info]

Example (to start PoseidonOS):
  poseidonos-cli system start
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	SystemCmd.AddCommand(StartSystemCmd)
	SystemCmd.AddCommand(StopSystemCmd)
	SystemCmd.AddCommand(SystemInfoCmd)
	SystemCmd.AddCommand(SetSystemPropCmd)
}
