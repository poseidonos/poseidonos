package arraycmds

import (
	"encoding/json"
	"os"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var UnmountArrayCmd = &cobra.Command{
	Use:   "unmount [flags]",
	Short: "Unmount an array from PoseidonOS.",
	Long: `
Unmount an array from PoseidonOS.

Syntax:
	unmount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array unmount --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: After unmounting array" + " " +
			unmount_array_arrayName + "," + " " +
			"all the volumes in the array will be unmounted.\n" +
			"In addition, progressing I/Os may fail if any.\n\n" +
			"Are you sure you want to unmount array" + " " +
			unmount_array_arrayName + "?"

		if unmount_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "UNMOUNTARRAY"

		param := messages.UnmountArrayParam{
			ARRAYNAME: unmount_array_arrayName,
		}

		uuid := globals.GenerateUUID()

		req := messages.BuildReqWithParam(command, uuid, param)

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
var unmount_array_arrayName = ""
var unmount_array_isForced = false

func init() {
	UnmountArrayCmd.Flags().StringVarP(&unmount_array_arrayName,
		"array-name", "a", "",
		"The name of the array to unmount.")
	UnmountArrayCmd.MarkFlagRequired("array-name")

	UnmountArrayCmd.Flags().BoolVarP(&unmount_array_isForced,
		"force", "", false,
		"Force to unmount this array.")
}
