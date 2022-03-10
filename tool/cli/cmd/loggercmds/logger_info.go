package loggercmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var LoggerInfoCmd = &cobra.Command{
	Use:   "info",
	Short: "Display the current preference of logger.",
	Long: `
Display the current preference of logger.

Syntax:
	poseidonos-cli logger info
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LOGGERINFO"

		uuid := globals.GenerateUUID()

		logerInfoReq := messages.BuildReq(command, uuid)

		reqJSON, err := json.Marshal(logerInfoReq)
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
