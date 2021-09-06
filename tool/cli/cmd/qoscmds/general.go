package qoscmds

import (
	"github.com/spf13/cobra"
)

var QosCmd = &cobra.Command{
	Use:   "qos",
	Short: "QoS commands for PoseidonOS.",
	Long: `
QoS commands for PoseidonOS. Use this command category when you want
to specify or display the quality-of-service of a specific volume. 

Syntax: 
  poseidonos-cli qos [create|reset|list] [flags]

Example:
1. To create a qos policy for a volume
  poseidonos-cli qos create --volume-name Volume0 --array-name Array0 --maxiops 1000 --maxbw 100
  
2. To create a qos policy for multiple volumes
  poseidonos-cli qos create --volume-name Volume0,Volume1,Volume2 --array-name Array0 --maxiops 1000 --maxbw 100
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	QosCmd.AddCommand(VolumePolicyCmd)
	QosCmd.AddCommand(VolumeResetCmd)
	QosCmd.AddCommand(ListQosCmd)
}
