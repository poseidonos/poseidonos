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
var SetSystemPropCmd = &cobra.Command{
	Use:   "set-property",
	Short: "Set the property of PoseidonOS.",
	Long: `
Set the property of PoseidonOS. 

Syntax:
	poseidonos-cli system set-property [--rebuild-impact (highest | higher | high | medium | low | lower | lowest)]

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact low.
          `,
	Run: func(cmd *cobra.Command, args []string) {
		// TODO(mj): now the message format is for REBUILDPERFIMPACT only.
		// The message format should be extended for other properties also.

		var command = "REBUILDPERFIMPACT"

		param := messages.SetSystemPropReqParam{
			LEVEL: set_system_property_level,
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var set_system_property_level = ""

func init() {
	SetSystemPropCmd.Flags().StringVarP(&set_system_property_level,
		"rebuild-impact", "", "",
		`The impact of rebuilding process on the I/O performance.
With high rebuilding-impact, the rebuilding process may
interfere with I/O operations more. Therefore, I/O operations may
slow down although rebuilding process becomes accelerated. 
`)
}
