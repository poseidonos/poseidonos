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
			STRUCTUREDLOGGING: set_pref_req_strLogging,
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

var (
	set_pref_req_strLogging = ""
)

func init() {
	SetPrefCmd.Flags().StringVarP(&set_pref_req_strLogging,
		"structured-logging", "s", "false",
		`When specified as true, PoseidonOS will log the events in JSON form for structured logging.
Otherwise, the events will be logged in plain text form.`)
	SetPrefCmd.MarkFlagRequired("level")
}
