package loggercmds

import (
	"github.com/spf13/cobra"
)

var LoggerCmd = &cobra.Command{
	Use:   "logger",
	Short: "Logger commands for PoseidonOS.",
	Long: `Logger commands for PoseidonOS.

Syntax: 
  poseidonos-cli logger [set-level|get-level|apply-filter|info]

Example (to get the current log level):
  poseidonos-cli logger get-level
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	LoggerCmd.AddCommand(SetLevelCmd)
	LoggerCmd.AddCommand(GetLevelCmd)
	LoggerCmd.AddCommand(ApplyFilterCmd)
	LoggerCmd.AddCommand(LoggerInfoCmd)
}
