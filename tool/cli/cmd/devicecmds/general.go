package devicecmds

import (
	"github.com/spf13/cobra"
)

var DeviceCmd = &cobra.Command{
	Use:   "device",
	Short: "Device commands for PoseidonOS.",
	Long: `
Device commands for PoseidonOS. Use this command category to create,
delete, and display devices. 

Syntax: 
  poseidonos-cli device [create|scan|list|smart] [flags]

Example (to scan devices in the system):
  poseidonos-cli device scan
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	DeviceCmd.AddCommand(CreateDeviceCmd)
	DeviceCmd.AddCommand(ScanDeviceCmd)
	DeviceCmd.AddCommand(ListDeviceCmd)
	DeviceCmd.AddCommand(SMARTLOGCmd)
}
