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

var SetPrefCmd = &cobra.Command{
	Use:   "set-preference",
	Short: "Set the preferences (e.g., format) of logger.",
	Long: `
Set the preferences (e.g., format) of logger.

Syntax:
	poseidonos-cli logger set-preference [--json BooleanValue]
          `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "SETLOGPREFERENCE"

		param := messages.SetPrefReqParam{
			LOGJSON: set_pref_req_logJson,
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

var (
	set_pref_req_logJson = ""
)

func init() {
	SetPrefCmd.Flags().StringVarP(&set_pref_req_logJson,
		"log-json", "j", "false",
		`When specified as true, PoseidonOS will log the events in JSON form.
Otherwise, the events will be logged in plain text form.`)
	SetPrefCmd.MarkFlagRequired("level")
}
