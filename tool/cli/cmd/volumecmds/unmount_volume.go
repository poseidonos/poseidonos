package volumecmds

import (
	"encoding/json"
	"os"
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
	unmount (--volume-name | -v) VolumeName (--array-name | -a) ArrayName [--subnqn TargetNVMSubsystemNVMeQualifiedName]

Example: 
	poseidonos-cli volume unmount --volume-name Volume0 --array-name Volume0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: After unmounting volume" + " " +
			unmount_volume_volumeName + " " +
			"in array " + unmount_volume_arrayName + "," + " " +
			"the progressing I/Os may fail if any.\n\n" +
			"Are you sure you want to unmount volume" + " " +
			unmount_volume_volumeName + "?"

		if unmount_volume_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

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

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Debug("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var unmount_volume_volumeName = ""
var unmount_volume_arrayName = ""
var unmount_volume_isForced = false

func init() {
	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_volumeName, "volume-name", "v", "", "The name of the volume to unmount")
	UnmountVolumeCmd.MarkFlagRequired("volume-name")

	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_arrayName, "array-name", "a", "", "The name of the array where the volume belongs to")
	UnmountVolumeCmd.MarkFlagRequired("array-name")

	UnmountVolumeCmd.Flags().BoolVarP(&unmount_volume_isForced, "force", "", false, "Execute this command without confirmation")
}
