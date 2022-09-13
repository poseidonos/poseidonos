package arraycmds

import (
	"strings"

	"github.com/spf13/cobra"
)

var maxNumDataDevsNoRaid = 1

// This is a pseudo command to contain array subcommands.
// ArryCmd itself does nothing when called.
var ArrayCmd = &cobra.Command{
	Use:   "array",
	Short: "Array command for PoseidonOS.",
	Long: `Array command for PoseidonOS. Use this command to create, delete, and control arrays.

Syntax: 
  poseidonos-cli array [create|delete|mount|unmount|list|addspare|rmspare|autocreate] [flags]

Example (to create an array):
  poseidonos-cli array create --array-name array0 --buffer uram0 --data-devs nvme0,nvme1,nvme2,nvme3 --spare nvme4
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	// Add subcommands to array command.
	// If you create a new subcommand, add it here.
	ArrayCmd.AddCommand(ListArrayCmd)
	ArrayCmd.AddCommand(MountArrayCmd)
	ArrayCmd.AddCommand(UnmountArrayCmd)
	ArrayCmd.AddCommand(DeleteArrayCmd)
	ArrayCmd.AddCommand(CreateArrayCmd)
	ArrayCmd.AddCommand(AddSpareCmd)
	ArrayCmd.AddCommand(RemoveSpareCmd)
	ArrayCmd.AddCommand(ReplaceArrayDeviceCmd)
	ArrayCmd.AddCommand(AutocreateArrayCmd)
	ArrayCmd.AddCommand(RebuildArrayCmd)
}

func isRAIDConstMet(numOfDataDevs int, raid string) bool {
	if (strings.ToLower(raid) == "raid10") && (numOfDataDevs%2 == 1) {
		return false
	}

	return true
}
