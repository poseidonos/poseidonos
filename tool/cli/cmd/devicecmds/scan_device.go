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
	Long: `
Scan devices in the system. Use this command when PoseidonOS has 
(re)started.

Syntax:
	poseidonos-cli device scan .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SCANDEVICE"

		uuid := globals.GenerateUUID()

		req := messages.Request{
			RID:     uuid,
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Error("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
