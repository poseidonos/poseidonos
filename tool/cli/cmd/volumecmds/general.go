package volumecmds

import (
	"github.com/spf13/cobra"
)

var VolumeCmd = &cobra.Command{
	Use:   "volume",
	Short: "Volume commands for PoseidonOS.",
	Long: `
	Volume commands for PoseidonOS. Use this command category to control volumes
	or display the information of the volumes. 

Syntax: 
  poseidonos-cli volume [create|delete|mount|unmount|list|rename|mount-with-subsystem] [flags]

Example (to create a volume):
  poseidonos-cli volume create --volume-name Volume0 --array-name Array0 
  --size 1024GB --maxiops 1000 --maxbw 100GB/s
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	VolumeCmd.AddCommand(CreateVolumeCmd)
	VolumeCmd.AddCommand(DeleteVolumeCmd)
	VolumeCmd.AddCommand(ListVolumeCmd)
	VolumeCmd.AddCommand(MountVolumeCmd)
	VolumeCmd.AddCommand(UnmountVolumeCmd)
	VolumeCmd.AddCommand(RenameVolumeCmd)
	VolumeCmd.AddCommand(MountVolumeWithSubsystemCmd)
	VolumeCmd.AddCommand(SetVolumePropertyCmd)
}
