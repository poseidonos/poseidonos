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

var DeleteVolumeCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete a volume from PoseidonOS.",
	Long: `Delete a volume from PoseidonOS.

Syntax:
	poseidonos-cli volume delete (--volume-name | -v) VolumeName --array-name ArrayName

Example: 
	poseidonos-cli volume delete --volume-name Volume0 --array=name Array0
	
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "DELETEVOLUME"

		deleteVolumeParam := messages.DeleteVolumeParam{
			VOLUMENAME: delete_volume_volumeName,
			ARRAYNAME:  delete_volume_arrayName,
		}

		deleteVolumeReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   deleteVolumeParam,
		}

		reqJSON, err := json.Marshal(deleteVolumeReq)
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
var delete_volume_volumeName = ""
var delete_volume_arrayName = ""

func init() {
	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_volumeName, "volume-name", "v", "", "The Name of the volume to delete")
	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_arrayName, "array-name", "a", "", "The Name of the array where the volume belongs to")
}
