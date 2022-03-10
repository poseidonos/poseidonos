package devicecmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var SMARTLOGCmd = &cobra.Command{
	Use:   "smart-log",
	Short: "Display SMART log information of a device.",
	Long: `
Display SMART log information of a device.

Syntax:
	poseidonos-cli device smart-log (--device-name | -d) DeviceName
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SMARTLOG"

		param := messages.SMARTLOGReqParam{
			DEVICENAME: smart_log_deviceName,
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
var smart_log_deviceName = ""

func init() {
	SMARTLOGCmd.Flags().StringVarP(&smart_log_deviceName,
		"device-name", "d", "",
		"The name of the device to display the SMART log information.")
	SMARTLOGCmd.MarkFlagRequired("device-name")
}
