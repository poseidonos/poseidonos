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

var ListSubsystemCmd = &cobra.Command{
	Use:   "list",
	Short: "List subsystems from PoseidonOS.",
	Long: `
List subsystems from PoseidonOS.

Syntax:
	poseidonos-cli subsystem list [(--subnqn | -q) SubsystemNQN]

Example 1 (listing all subsystems):
	poseidonos-cli subsystem list

Example 2 (listing a specific array):
	poseidonos-cli subsystem list --subnqn nqn.2019-04.pos:subsystem
    `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = ""
		var ListSubsystemReq = messages.Request{}
		if list_subsystem_subnqn == "" {
			command = "LISTSUBSYSTEM"

			uuid := globals.GenerateUUID()

			ListSubsystemReq = messages.BuildReq(command, uuid)
		} else {
			command = "SUBSYSTEMINFO"
			param := messages.ListSubsystemParam{
				SUBNQN: list_subsystem_subnqn,
			}

			uuid := globals.GenerateUUID()

			ListSubsystemReq = messages.BuildReqWithParam(command, uuid, param)
		}

		reqJSON, err := json.Marshal(ListSubsystemReq)
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
var list_subsystem_subnqn = ""

func init() {
	ListSubsystemCmd.Flags().StringVarP(&list_subsystem_subnqn,
		"subnqn", "q", "",
		"NQN of the subsystem to list spec")
}
