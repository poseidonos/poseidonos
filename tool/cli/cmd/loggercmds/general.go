package loggercmds

import (
	"github.com/spf13/cobra"
)

var LoggerCmd = &cobra.Command{
	Use:   "logger",
	Short: "Logger commands for PoseidonOS.",
	Long: `
Logger commands for PoseidonOS. Use this command category to
control and display information about logger. 

Syntax: 
  poseidonos-cli logger [set-level|get-level|apply-filter|info|set-preference] [flags]

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
	LoggerCmd.AddCommand(SetPrefCmd)
}
