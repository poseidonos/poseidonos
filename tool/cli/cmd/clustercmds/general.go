package clustercmds

import (
	"cli/cmd/globals"

	"github.com/spf13/cobra"
)

var ClusterCmd = &cobra.Command{
	Use:   "cluster",
	Short: "Cluster commands for PoseidonOS.",
	Long: `
Cluster commands for PoseidonOS. Use this command category to manage high-availability clusters.

Note: before using this command, you need to set the database information about high-availability 
clusters (IP address, port number, username, password, and database name) as environmental variables.

The variables to set are:
  ` + globals.HaDbIPVar + " " + globals.HaDbNameVar + " " + globals.HaDbUserVar + " " + globals.HaDbPasswordVar + " " + globals.HaDbNameVar +
		`

Syntax: 
  poseidonos-cli cluster [ln] [flags]

Example (to list nodes in the system):
  poseidonos-cli cluster ln
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	ClusterCmd.AddCommand(ListNodeCmd)
	ClusterCmd.AddCommand(ListHaVolumeCmd)
	ClusterCmd.AddCommand(ListHaReplicationCmd)
	ClusterCmd.AddCommand(StartHaReplicationCmd)
}
