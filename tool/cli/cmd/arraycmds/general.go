package arraycmds

import (
	"cli/cmd/messages"
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
	ArrayCmd.AddCommand(AutocreateArrayCmd)
}

// Parse comma-separated device list string and return the device list
func parseDevList(devsList string) []messages.DeviceNameList {

	if devsList == "" {
		return nil
	}

	devsListSlice := strings.Split(devsList, ",")

	var devs []messages.DeviceNameList
	for _, str := range devsListSlice {
		var devNameList messages.DeviceNameList // Single device name that is splitted
		devNameList.DEVICENAME = str
		devs = append(devs, devNameList)
	}

	return devs
}
