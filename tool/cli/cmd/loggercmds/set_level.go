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

var SetLevelCmd = &cobra.Command{
	Use:   "set-level",
	Short: "Set the filtering level of logger.",
	Long: `
Set the filtering level of logger.

Syntax:
	poseidonos-cli logger set-level --level [debug | info | warning | error | critical]
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SETLOGLEVEL"

		param := messages.SetLevelReqParam{
			LEVEL: set_level_loggerLevel,
		}

		uuid := globals.GenerateUUID()

		setLevelReq := messages.BuildReqWithParam(command, uuid, param)

		reqJSON, err := json.Marshal(setLevelReq)
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var set_level_loggerLevel = ""

func init() {
	SetLevelCmd.Flags().StringVarP(&set_level_loggerLevel,
		"level", "", "",
		`The level of logger to set.
	
- critical: events that make the system not available.
- error: events when PoseidonOS cannot process the request
	because of an internal problem. 
- warning: events when unexpected user input has been detected. 
- debug: logs when the system is working properly.
- info: logs for debug binary.`)
	SetLevelCmd.MarkFlagRequired("level")
}
