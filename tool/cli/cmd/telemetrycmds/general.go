package telemetrycmds

import (
	"github.com/spf13/cobra"
)

var TelemetryCmd = &cobra.Command{
	Use:   "telemetry",
	Short: "Telemetry commands for PoseidonOS.",
	Long: `
Telemetry commands for PoseidonOS. Use this command category to
start/stop or configure the telemetry of PoseidonOS. For example,
PoseidonOS will not gather the internal statistics once you execute
the temeletry stop command.  

Syntax: 
  poseidonos-cli telemetry [start|stop|set-property]

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
	TelemetryCmd.AddCommand(SetTelemetryPropCmd)
	TelemetryCmd.AddCommand(GetTelemetryPropCmd)
}
