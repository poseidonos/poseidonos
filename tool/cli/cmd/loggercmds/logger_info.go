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
	Short: "Get the current settings of the logger.",
	Long: `
Get the current settings of the logger.

Syntax:
	poseidonos-cli logger info .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LOGGERINFO"

		uuid := globals.GenerateUUID()

		logerInfoReq := messages.Request{
			RID:     uuid,
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(logerInfoReq)
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
