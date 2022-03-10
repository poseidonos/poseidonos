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

var DeleteVolumeCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete a volume from PoseidonOS.",
	Long: `
Delete a volume from an array in PoseidonOS.

Syntax:
	poseidonos-cli volume delete (--volume-name | -v) VolumeName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli volume delete --volume-name Volume0 --array=name Array0
	
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: After deleting volume" + " " +
			delete_volume_volumeName + "," + " " +
			"you cannot recover the data of volume " +
			delete_volume_volumeName + " " +
			"in the array " +
			delete_volume_arrayName + "\n\n" +
			"Are you sure you want to delete volume" + " " +
			delete_volume_volumeName + "?"

		if delete_volume_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETEVOLUME"

		param := messages.DeleteVolumeParam{
			VOLUMENAME: delete_volume_volumeName,
			ARRAYNAME:  delete_volume_arrayName,
		}

		uuid := globals.GenerateUUID()

		deleteVolumeReq := messages.BuildReqWithParam(command, uuid, param)

		reqJSON, err := json.Marshal(deleteVolumeReq)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var delete_volume_volumeName = ""
var delete_volume_arrayName = ""
var delete_volume_isForced = false

func init() {
	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_volumeName,
		"volume-name", "v", "",
		"The Name of the volume to delete.")
	DeleteVolumeCmd.MarkFlagRequired("volume-name")

	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_arrayName,
		"array-name", "a", "",
		"The Name of the array where the volume belongs to.")
	DeleteVolumeCmd.MarkFlagRequired("array-name")

	DeleteVolumeCmd.Flags().BoolVarP(&delete_volume_isForced,
		"force", "", false,
		"Force to delete the volume (volume must be unmounted first).")
}
