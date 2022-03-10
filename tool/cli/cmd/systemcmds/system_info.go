package systemcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var SystemInfoCmd = &cobra.Command{
	Use:   "info",
	Short: "Display information of PoseidonOS.",
	Long: `
Display the information of PoseidonOS.

Syntax:
	poseidonos-cli system info
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "GETPOSINFO"

		uuid := globals.GenerateUUID()

		req := messages.BuildReq(command, uuid)

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

func init() {

}
