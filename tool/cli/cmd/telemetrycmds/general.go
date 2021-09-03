package telemetrycmds

import (
	"github.com/spf13/cobra"
)

var TelemetryCmd = &cobra.Command{
	Use:   "telemetry",
	Short: "Telemetry commands for PoseidonOS.",
	Long: `Telemetry commands for PoseidonOS.

Syntax: 
  poseidonos-cli telemetry [start|stop]

Example (to start telemetry PoseidonOS):
  poseidonos-cli telemetry start
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	TelemetryCmd.AddCommand(StartTelemetryCmd)
	TelemetryCmd.AddCommand(StopTelemetryCmd)

}
