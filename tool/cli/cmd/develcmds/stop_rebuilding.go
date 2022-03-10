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
var StopRebuildingCmd = &cobra.Command{
	Use:   "stop-rebuilding",
	Short: "Stop rebulding.",
	Long: `
Stop rebuilding.

Syntax:
	poseidonos-cli devel stop-rebuilding ( --array-name | -a ) ArrayName
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STOPREBUILDING"

		param := messages.StopRebuildingParam{
			ARRAYNAME: stop_rebuilding_arrayName,
		}

		uuid := globals.GenerateUUID()

		req := messages.Request{
			RID:       uuid,
			COMMAND:   command,
			PARAM:     param,
			REQUESTOR: "cli",
		}

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

var stop_rebuilding_arrayName = ""

func init() {
	StopRebuildingCmd.Flags().StringVarP(&stop_rebuilding_arrayName,
		"array-name", "a", "", "Array name.")
	StopRebuildingCmd.MarkFlagRequired("array-name")
}
