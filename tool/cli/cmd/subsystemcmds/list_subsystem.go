package subsystemcmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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

Example 2 (listing a specific subsystem):
	poseidonos-cli subsystem list --subnqn nqn.2019-04.pos:subsystem
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command string

		if list_subsystem_subnqn != "" {
			command = "SUBSYSTEMINFO"
			executeSubsystemInfoCmd(command)
		} else {
			command = "LISTSUBSYSTEM"
			executeListSubsystemCmd(command)
		}
	},
}

func executeSubsystemInfoCmd(command string) {
	uuid := globals.GenerateUUID()
	param := &pb.SubsystemInfoRequest_Param{Subnqn: list_subsystem_subnqn}
	req := &pb.SubsystemInfoRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
			res, err := grpcmgr.SendSubsystemInfo(req)
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
}

func executeListSubsystemCmd(command string) {
	uuid := globals.GenerateUUID()
	req := &pb.ListSubsystemRequest{Command: command, Rid: uuid, Requestor: "cli"}

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
			res, err := grpcmgr.SendListSubsystem(req)
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
