package systemcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

// TODO(mj): Currently, this command only supports REBUILDPERFIMPACT command.
// This command should be extended to support other commands to set the property of PoseidonOS as well.
var GetSystemPropCmd = &cobra.Command{
	Use:   "get-property",
	Short: "Display the property of PoseidonOS.",
	Long: `
Display the property (e.g., rebuilding performance impact) of PoseidonOS. 

Syntax:
	poseidonos-cli system get-property

Example:
	poseidonos-cli system get-property
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "GETSYSTEMPROPERTY"

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
