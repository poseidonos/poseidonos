package subsystemcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var DeleteSubsystemCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete a subsystem from PoseidonOS.",
	Long: `Delete a subsystem from PoseidonOS.

Syntax:
	poseidonos-cli subsystem delete --subnqn SubsystemNQN

Example:
	poseidonos-cli subsystem delete --subnqn nqn.2019-04.pos:subsystem
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "DELETESUBSYSTEM"

		deleteSubsystemParam := messages.DeleteSubsystemParam{
			SUBNQN: subsystem_delete_subnqn,
		}

		req := messages.Request{
			RID:     "fromCLI",
			COMMAND: command,
			PARAM:   deleteSubsystemParam,
		}

		reqJSON, err := json.Marshal(req)
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
var subsystem_delete_subnqn = ""

func init() {
	DeleteSubsystemCmd.Flags().StringVarP(&subsystem_delete_subnqn, "subnqn", "a", "", "NQN of the subsystem to delete")
}
