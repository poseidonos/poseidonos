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
	Long: `Get the current settings of the logger.

Syntax:
	poseidonos-cli logger info .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LOGGERINFO"

		logerInfoReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(logerInfoReq)
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
