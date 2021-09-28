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

var DeleteArrayCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete an array from PoseidonOS.",
	Long: `
Delete an array from PoseidonOS. After executing this command, 
the data and volumes in the array will be deleted too.

Syntax:
	poseidonos-cli array delete (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array delete --array-name Array0	
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: After deleting array" + " " +
			delete_array_arrayName + "," + " " +
			"you cannot recover the data of the volumes in the array.\n\n" +
			"Are you sure you want to delete array" + " " +
			delete_array_arrayName + "?"

		if delete_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETEARRAY"

		deleteArrayParam := messages.DeleteArrayParam{
			ARRAYNAME: delete_array_arrayName,
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

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Debug("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var delete_array_arrayName = ""
var delete_array_isForced = false

func init() {
	DeleteArrayCmd.Flags().StringVarP(&delete_array_arrayName,
		"array-name", "a", "", "The name of the array to delete")
	DeleteArrayCmd.MarkFlagRequired("array-name")

	DeleteArrayCmd.Flags().BoolVarP(&delete_array_isForced,
		"force", "", false,
		"Force to delete this array (array must be unmounted first).")
}
