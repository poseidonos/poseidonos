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
var UpdateEventWrrCmd = &cobra.Command{
	Use:   "update-event-wrr",
	Short: "Set the weights for backend events such as Flush, Rebuild, and GC.",
	Long: `
Set the weights for backend events such as Flush, Rebuild, and GC.

Syntax:
	poseidonos-cli devel update-event-wrr --name ( flush | fe_rebuild | rebuild | gc ) --weight ( 1 | 2 | 3 )
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "UPDATEEVENTWRRPOLICY"

		param := messages.UpdateEventWrrParam{
			NAME:   update_event_wrr_name,
			WEIGHT: update_event_wrr_weight,
		}

		uuid := globals.GenerateUUID()

		req := messages.BuildReqWithParam(command, uuid, param)

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

var default_weight = 20
var update_event_wrr_name = ""
var update_event_wrr_weight = default_weight

func init() {
	UpdateEventWrrCmd.Flags().StringVarP(&update_event_wrr_name,
		"name", "", "", "Event name.")
	UpdateEventWrrCmd.MarkFlagRequired("name")

	UpdateEventWrrCmd.Flags().IntVarP(&update_event_wrr_weight,
		"weight", "", default_weight, "Weight.")
	UpdateEventWrrCmd.MarkFlagRequired("weight")
}
