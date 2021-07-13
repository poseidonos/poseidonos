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
	Long: `Set the filtering level of logger.

Syntax:
	poseidonos-cli logger set-level --level [error | debug | warn | err | critical]
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SETLOGLEVEL"

		setLevelReqParam := messages.SetLevelReqParam{
			LEVEL: set_level_loggerLevel,
		}

		setLevelReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   setLevelReqParam,
		}

		reqJSON, err := json.Marshal(setLevelReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Debug("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var set_level_loggerLevel = ""

func init() {
	//LogLevel = "error" | "debug" | "warn" | "err" | "critical"
	SetLevelCmd.Flags().StringVarP(&set_level_loggerLevel, "level", "", "", "The level of logger to set")
}
