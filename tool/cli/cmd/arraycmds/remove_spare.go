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
	Long: `
Remove a spare device from an array of PoseidonOS.

Syntax:
	poseidonos-cli array rmspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rmspare --spare DeviceName --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "REMOVEDEVICE"

		var spareDevName [1]messages.SpareDeviceName
		spareDevName[0].SPARENAME = remove_spare_spareDevName

		param := messages.SpareParam{
			ARRAYNAME: remove_spare_arrayName,
			SPARENAME: spareDevName,
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
var remove_spare_spareDevName = ""
var remove_spare_arrayName = ""

func init() {
	RemoveSpareCmd.Flags().StringVarP(&remove_spare_arrayName, "array-name",
		"a", "",
		"The name of the array to remove the specified spare device.")
	RemoveSpareCmd.MarkFlagRequired("array-name")

	RemoveSpareCmd.Flags().StringVarP(&remove_spare_spareDevName,
		"spare", "s", "",
		"The name of the device to remove from the array.")
	RemoveSpareCmd.MarkFlagRequired("spare")
}
