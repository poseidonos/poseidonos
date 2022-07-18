package subsystemcmds

import (
	"os"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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
		uuid := globals.GenerateUUID()
		param := &pb.DeleteSubsystemRequest_Param{
			Subnqn: delete_subsystem_subnqn,
		}

		req := &pb.DeleteSubsystemRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			var resJSON string

			if globals.EnableGrpc == false {
				resJSON = socketmgr.SendReqAndReceiveRes(string(reqJSON))
			} else {
				res, err := grpcmgr.SendDeleteSubsystem(req)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJSON = string(resByte)
			}

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
