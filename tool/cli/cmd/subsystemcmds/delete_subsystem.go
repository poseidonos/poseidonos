package subsystemcmds

import (
	"encoding/json"
	"os"

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
	Long: `
Delete a subsystem from PoseidonOS.

Syntax:
	poseidonos-cli subsystem delete (--subnqn | -q) SubsystemNQN

Example:
	poseidonos-cli subsystem delete --subnqn nqn.2019-04.pos:subsystem
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "Are you sure " +
			"you want to delete subsystem" + " " +
			delete_subsystem_subnqn + "?"

		if delete_subsystem_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETESUBSYSTEM"

		param := messages.DeleteSubsystemParam{
			SUBNQN: delete_subsystem_subnqn,
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
var delete_subsystem_subnqn = ""
var delete_subsystem_isForced = false

func init() {
	DeleteSubsystemCmd.Flags().StringVarP(&delete_subsystem_subnqn,
		"subnqn", "q", "",
		"NQN of the subsystem to delete.")
	DeleteSubsystemCmd.MarkFlagRequired("subnqn")

	DeleteSubsystemCmd.Flags().BoolVarP(&delete_subsystem_isForced,
		"force", "", false, "Force to delete this subsystem.")
}
