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

var SystemInfoCmd = &cobra.Command{
	Use:   "info",
	Short: "Show information of PoseidonOS.",
	Long: `Show information of PoseidonOS.

Syntax:
	poseidonos-cli system stop .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "GETIBOFOSINFO"

		systemInfoReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(systemInfoReq)
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
