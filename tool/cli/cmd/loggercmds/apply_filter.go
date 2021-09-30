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

var ApplyFilterCmd = &cobra.Command{
	Use:   "apply-filter",
	Short: "Apply a filtering policy to the logger.",
	Long: `
Apply a filtering policy to the logger.

Syntax:
	poseidonos-cli logger apply-filter .
          `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "APPLYLOGFILTER"

		applyFilterReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(applyFilterReq)
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
