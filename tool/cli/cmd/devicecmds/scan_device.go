package devicecmds

import (
	"encoding/json"
	"fmt"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

//TODO(mj): function for --detail flag needs to be implemented.
var ScanDeviceCmd = &cobra.Command{
	Use:   "scan",
	Short: "Scan devices in the system.",
	Long: `Scan devices in the system.

Syntax:
	poseidonos-cli device scan .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SCANDEVICE"

		scanDeviceReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(scanDeviceReq)
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

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
