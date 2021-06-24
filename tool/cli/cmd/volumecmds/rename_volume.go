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

//TODO(mj): function for --detail flag needs to be implemented.
var RenameVolumeCmd = &cobra.Command{
	Use:   "rename [flags]",
	Short: "Rename a volume of PoseidonOS.",
	Long: `Rename a volume of PoseidonOS.

Syntax:
	poseidonos-cli volume rename (--volume-name | -v) VolumeName --array-name ArrayName (--new-volume-name | -n) VolumeName .

Example (renaming a volume): 
	poseidonos-cli volume rename --volume-name OldVolumeName --array-name Array0 --new-volume-name NewVolumeName
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "RENAMEVOLUME"

		renameVolumeParam := messages.RenameVolumeParam{
			ARRAYNAME:     rename_volume_arrayName,
			VOLUMENAME:    rename_volume_volumeName,
			NEWVOLUMENAME: rename_volume_newVolumeName,
		}

		renameVolumeReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   renameVolumeParam,
		}

		reqJSON, err := json.Marshal(renameVolumeReq)
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
var rename_volume_arrayName = ""
var rename_volume_volumeName = ""
var rename_volume_newVolumeName = ""

func init() {
	RenameVolumeCmd.Flags().StringVarP(&rename_volume_arrayName, "array-name", "a", "", "The Name of the array of volumes to list")
	RenameVolumeCmd.Flags().StringVarP(&rename_volume_volumeName, "volume-name", "v", "", "The Name of the array of volumes to list")
	RenameVolumeCmd.Flags().StringVarP(&rename_volume_newVolumeName, "new-volume-name", "n", "", "The Name of the array of volumes to list")
}
