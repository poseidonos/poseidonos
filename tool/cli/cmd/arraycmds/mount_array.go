package arraycmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var MountArrayCmd = &cobra.Command{
	Use:   "mount [flags]",
	Short: "Mount an array to PoseidonOS.",
	Long: `Mount an array to PoseidonOS.

Syntax:
	mount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array mount --array-name Array0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "MOUNTARRAY"

		mountArrayParam := messages.MountArrayParam{
			ARRAYNAME: mount_array_arrayName,
		}

		mountArrayReq := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   mountArrayParam,
		}

		reqJSON, err := json.Marshal(mountArrayReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_array_arrayName = ""

func init() {
	MountArrayCmd.Flags().StringVarP(&mount_array_arrayName, "array-name", "a", "", "The name of the array to mount")
}
