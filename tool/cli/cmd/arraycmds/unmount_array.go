package arraycmds

import (
	"encoding/json"

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
	Long: `Unmount an array from PoseidonOS.

Syntax:
	unmount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array unmount --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "UNMOUNTARRAY"

		unmountArrayParam := messages.UnmountArrayParam{
			ARRAYNAME: unmount_array_arrayName,
		}

		unmountArrayReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   unmountArrayParam,
		}

		reqJSON, err := json.Marshal(unmountArrayReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var unmount_array_arrayName = ""

func init() {
	UnmountArrayCmd.Flags().StringVarP(&unmount_array_arrayName, "array-name", "a", "", "The name of the array to unmount")
}
