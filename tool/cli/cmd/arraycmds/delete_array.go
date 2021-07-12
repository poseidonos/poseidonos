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

var DeleteArrayCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete an array from PoseidonOS.",
	Long: `Delete an array from PoseidonOS.

Syntax:
	poseidonos-cli array delete (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array delete --array-name Array0	
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "DELETEARRAY"

		deleteArrayParam := messages.DeleteArrayParam{
			ARRAYNAME: array_delete_arrayName,
		}

		req := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   deleteArrayParam,
		}

		reqJSON, err := json.Marshal(req)
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
var array_delete_arrayName = ""

func init() {
	DeleteArrayCmd.Flags().StringVarP(&array_delete_arrayName, "array-name", "a", "", "Name of the array to delete")
}
