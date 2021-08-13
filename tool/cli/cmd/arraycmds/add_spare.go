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

var AddSpareCmd = &cobra.Command{
	Use:   "addspare [flags]",
	Short: "Add a device as spare to an array.",
	Long: `Add a device as spare to an array.

Syntax:
	poseidonos-cli array addspare (--spare | -s) SpareDeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array addspare --spare nvme5 --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "ADDDEVICE"

		// Build a param for request
		var spareName [1]messages.SpareDeviceName
		spareName[0].SPARENAME = add_spare_spareDev

		addSpareParam := messages.SpareParam{
			ARRAYNAME: add_spare_arrayName,
			SPARENAME: spareName,
		}

		addSpareReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   addSpareParam,
		}

		reqJSON, err := json.Marshal(addSpareReq)
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
var add_spare_arrayName = ""
var add_spare_spareDev = ""

func init() {
	AddSpareCmd.Flags().StringVarP(&add_spare_arrayName, "array-name", "a", "", "Name of the array to add a spare device")
	AddSpareCmd.MarkFlagRequired("array-name")

	AddSpareCmd.Flags().StringVarP(&add_spare_spareDev, "spare", "s", "", "Name of the device to add to an array")
	AddSpareCmd.MarkFlagRequired("spare")
}
