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

var RemoveSpareCmd = &cobra.Command{
	Use:   "rmspare [flags]",
	Short: "Remove a spare device from an array of PoseidonOS.",
	Long: `Remove a spare device from an array of PoseidonOS.

Syntax:
	poseidonos-cli array rmspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rmspare --spare SpareDeviceName --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "REMOVEDEVICE"

		var spareDevName [1]messages.SpareDeviceName
		spareDevName[0].SPARENAME = remove_spare_spareDevName

		removeSpareParam := messages.SpareParam{
			ARRAYNAME: remove_spare_arrayName,
			SPARENAME: spareDevName,
		}

		removeSpareReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   removeSpareParam,
		}

		reqJSON, err := json.Marshal(removeSpareReq)
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
var remove_spare_spareDevName = ""
var remove_spare_arrayName = ""

func init() {
	RemoveSpareCmd.Flags().StringVarP(&remove_spare_arrayName, "array-name", "a", "", "Name of the array to remove a spare device")
	RemoveSpareCmd.Flags().StringVarP(&remove_spare_spareDevName, "spare", "s", "", "Name of the device to remove to an array")
}
