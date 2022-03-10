package develcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

//TODO(mj): function for --detail flag needs to be implemented.
var ResetEventWrrCmd = &cobra.Command{
	Use:   "reset-event-wrr",
	Short: "Reset the wieghts for backend events such as Flush, Rebuild, and GC to the default values.",
	Long: `
Reset the wieghts for backend events such as Flush, Rebuild, and GC to the default values.

Syntax:
	poseidonos-cli devel reset-event-wrr
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "RESETEVENTWRRPOLICY"

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
