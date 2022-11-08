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

var CreateSubsystemCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create an NVMe-oF subsystem to PoseidonOS.",
	Long: `Create an NVMe-oF subsystem to PoseidonOS.

Syntax:
	poseidonos-cli subsystem create (--subnqn | -q) SubsystemNQN 
	[--serial-number SerialNumber] [--model-number ModelNumber] 
	[(--max-namespaces | -m) MaxNamespace] [(--allow-any-host | -o)] [(--ana-reporting | -r)]

Example:
	poseidonos-cli subsystem create --subnqn nqn.2019-04.pos:subsystem1 
	--serial-number POS00000000000001 --model-number POS_VOLUME_EXTENSION -m 256 -o
    `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "CREATESUBSYSTEM"
		uuid := globals.GenerateUUID()
		param := &pb.CreateSubsystemRequest_Param{
			Nqn:           subsystem_create_subnqn,
			SerialNumber:  subsystem_create_serial,
			ModelNumber:   subsystem_create_model,
			MaxNamespaces: subsystem_create_maxnamespace,
			AllowAnyHost:  subsystem_create_allowanyhost,
			AnaReporting:  subsystem_create_anareporting,
		}

		req := &pb.CreateSubsystemRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
				res, err := grpcmgr.SendCreateSubsystem(req)
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
var (
	subsystem_create_subnqn              = ""
	subsystem_create_serial              = ""
	subsystem_create_model               = ""
	subsystem_create_maxnamespace uint32 = 0
	subsystem_create_allowanyhost        = false
	subsystem_create_anareporting        = false
)

func init() {
	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_subnqn, "subnqn", "q", "", "NQN of the subsystem to create")
	CreateSubsystemCmd.MarkFlagRequired("subnqn")

	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_serial, "serial-number", "", "", "Serial Number of the subsystem to create. Default : POS00000000000000")
	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_model, "model-number", "", "", "Model Number of the subsystem to create. Default : POS_VOLUME_EXTENTION")
	CreateSubsystemCmd.Flags().Uint32VarP(&subsystem_create_maxnamespace, "max-namespaces", "m", 0, "Maximum number of namespaces allowed. Default : 256")
	CreateSubsystemCmd.Flags().BoolVarP(&subsystem_create_allowanyhost, "allow-any-host", "o", false, "Allow any host to connect (don't enforce host NQN whitelist). Default : false")
	CreateSubsystemCmd.Flags().BoolVarP(&subsystem_create_anareporting, "ana-reporting", "r", false, "Enable ANA reporting feature. Default : false")
}
