package volumecmds

import (
	"encoding/json"
	"fmt"
	"pnconnector/src/log"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

//TODO(mj): function for --detail flag needs to be implemented.
var ListVolumeCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List volumes of an array or display information of a volume.",
	Long: `
List volumes of an array or display information of a volume.

Syntax:
	poseidonos-cli volume list (--array-name | -a) ArrayName [(--volume-name | -v) VolumeName]

Example1 (listing volumes of an array):
	poseidonos-cli volume list --array-name Array0

Example2 (displaying a detailed information of a volume):
	poseidonos-cli volume list --array-name Array0 --volume-name Volume0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var (
			command = ""
			param   interface{}
		)

		if list_volume_volumeName != "" {
			command = "VOLUMEINFO"

			param = messages.VolumeInfoParam{
				ARRAYNAME:  list_volume_arrayName,
				VOLUMENAME: list_volume_volumeName,
			}

		} else {
			command = "LISTVOLUME"

			param = messages.ListVolumeParam{
				ARRAYNAME: list_volume_arrayName,
			}

		}

		req := messages.BuildReqWithParam(command, globals.GenerateUUID(), param)

		reqJSON, err := json.Marshal(req)
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
var (
	list_volume_arrayName  = ""
	list_volume_volumeName = ""
)

func init() {
	ListVolumeCmd.Flags().StringVarP(&list_volume_arrayName,
		"array-name", "a", "",
		"The name of the array of volumes to list")
	ListVolumeCmd.MarkFlagRequired("array-name")

	ListVolumeCmd.Flags().StringVarP(&list_volume_volumeName,
		"volume-name", "v", "",
		"The name of the volume of the array to list."+"\n"+
			`When this is specified, the detailed information
		of this volume will be displayed.`)
}

func PrintResponse(response string) {
	fmt.Println(response)
}
