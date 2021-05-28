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
	Short: "List all volumes of an array.",
	Long: `List all volumes of an array.

Syntax:
	poseidonos-cli volume list [(--array-name | -a) ArrayName] .

Example (listing volumes from a specific array):
	poseidonos-cli volume list --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LISTVOLUME"

		listVolumeParam := messages.ListVolumeParam{
			ARRAYNAME: list_volume_arrayName,
		}

		listVolumeReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   listVolumeParam,
		}

		reqJSON, err := json.Marshal(listVolumeReq)
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
var list_volume_arrayName = ""

func init() {
	ListVolumeCmd.Flags().StringVarP(&list_volume_arrayName, "array-name", "a", "", "The Name of the array of volumes to list")
}

func PrintResponse(response string) {
	fmt.Println(response)
}
