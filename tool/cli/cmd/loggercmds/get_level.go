package loggercmds

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

var GetLevelCmd = &cobra.Command{
	Use:   "get-level",
	Short: "Get the filtering level of logger.",
	Long: `
Get the filtering level of logger.

Syntax:
	poseidonos-cli logger get-level .

          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "GETLOGLEVEL"

		getLevelReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(getLevelReq)
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
