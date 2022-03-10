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
	Short: "Apply a filtering policy to logger.",
	Long: `
Apply a filtering policy to logger.

Syntax:
	poseidonos-cli logger apply-filter
          `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "APPLYLOGFILTER"

		uuid := globals.GenerateUUID()

		applyFilterReq := messages.BuildReq(command, uuid)

		reqJSON, err := json.Marshal(applyFilterReq)
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
