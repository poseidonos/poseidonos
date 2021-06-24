package arraycmds

import (
	"github.com/spf13/cobra"
)

// This is a pseudo command to contain array subcommands.
// ArryCmd itself does nothing when called.
var ArrayCmd = &cobra.Command{
	Use:   "array",
	Short: "Array command for PoseidonOS.",
	Long: `Array command for PoseidonOS.

Syntax: 
  poseidonos-cli array [create|delete|mount|unmount|list|addspare|rmspare] [flags]

Example (to create an array):
  poseidonos-cli array create --array-name Array0 --buffer udev0 --data-devs nvme0,nvme1,nvme2,nvme3 --spare nvme4
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
}
