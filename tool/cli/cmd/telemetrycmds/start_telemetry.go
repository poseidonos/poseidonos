package telemetrycmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var StartTelemetryCmd = &cobra.Command{
	Use:   "start",
	Short: "Start the collection of telemetry data in PoseidonOS.",
	Long: `
Start the collection of telemetry data in PoseidonOS.

Syntax:
	poseidonos-cli telemetry start
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STARTTELEMETRY"

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
