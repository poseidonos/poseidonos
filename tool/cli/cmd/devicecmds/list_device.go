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

var ListDeviceCmd = &cobra.Command{
	Use:   "list",
	Short: "List all devices in the system.",
	Long: `List all devices in the system.

Syntax:
	poseidonos-cli device list .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LISTDEVICE"

		listDeviceReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(listDeviceReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		socketmgr.Connect()
		resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
		socketmgr.Close()

		displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
	},
}

func init() {

}
