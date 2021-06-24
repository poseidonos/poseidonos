package devicecmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var SMARTCmd = &cobra.Command{
	Use:   "smart",
	Short: "Show SMART information from an NVMe log page.",
	Long: `Show SMART information from an NVMe log page.

Syntax:
	poseidonos-cli device smart (--device-name | -d) DeviceName .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SMART"

		smartReqParam := messages.SMARTReqParam{
			DEVICENAME: smart_deviceName,
		}

		smartReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   smartReqParam,
		}

		reqJSON, err := json.Marshal(smartReq)
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var smart_deviceName = ""

func init() {
	SMARTCmd.Flags().StringVarP(&smart_deviceName, "device-name", "d", "", "The name of device to get the SMART information")
}
