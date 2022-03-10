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
var ResetMBRCmd = &cobra.Command{
	Use:   "resetmbr",
	Short: "Reset MBR information of PoseidonOS.",
	Long: `
Reset MBR information of PoseidonOS (Previously: Array Reset command).
Use this command when you need to remove the all the arrays and 
reset the states of the devices. 

Syntax:
	poseidonos-cli devel resetmbr
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "RESETMBR"

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
