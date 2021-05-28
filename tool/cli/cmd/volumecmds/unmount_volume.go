package volumecmds

import (
	"encoding/json"
	"pnconnector/src/log"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

var UnmountVolumeCmd = &cobra.Command{
	Use:   "unmount [flags]",
	Short: "Unmount a volume to Host.",
	Long: `Unmount a volume to Host.

Syntax:
	unmount --volume-name VolumeName (--array-name | -a) VolumeName [--subnqn TargetNVMSubsystemNVMeQualifiedName]

Example: 
	poseidonos-cli volume unmount --volume-name Volume0 --array-name Volume0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "UNMOUNTVOLUME"

		unmountVolumeParam := messages.UnmountVolumeParam{
			VOLUMENAME: unmount_volume_volumeName,
			ARRAYNAME:  unmount_volume_arrayName,
		}

		unmountVolumeReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   unmountVolumeParam,
		}

		reqJSON, err := json.Marshal(unmountVolumeReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		socketmgr.Connect()
		resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
		socketmgr.Close()

		displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var unmount_volume_volumeName = ""
var unmount_volume_arrayName = ""

func init() {
	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_volumeName, "volume-name", "", "", "The name of the volume to unmount")
	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_arrayName, "array-name", "a", "", "The name of the array where the volume belongs to")
}
