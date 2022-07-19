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

var AddListenerCmd = &cobra.Command{
	Use:   "add-listener [flags]",
	Short: "Add a listener to an NVMe-oF subsystem",
	Long: `
Add a listener to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem add-listener (--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId

Example:
	poseidonos-cli subsystem add-listener -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.100.2.14 -p 1158

    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "ADDLISTENER"
		uuid := globals.GenerateUUID()
		param := &pb.AddListenerRequest_Param{
			Subnqn:             subsystem_addlistener_subnqn,
			TransportType:      subsystem_addlistener_trtype,
			TargetAddress:      subsystem_addlistener_traddr,
			TransportServiceId: subsystem_addlistener_trsvcid,
		}

		req := &pb.AddListenerRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
				res, err := grpcmgr.SendAddListener(req)
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
var subsystem_addlistener_subnqn = ""
var subsystem_addlistener_trtype = ""
var subsystem_addlistener_traddr = ""
var subsystem_addlistener_trsvcid = ""

func init() {
	AddListenerCmd.Flags().StringVarP(&subsystem_addlistener_subnqn,
		"subnqn", "q", "",
		"The NQN of the subsystem to add listener.")
	AddListenerCmd.MarkFlagRequired("subnqn")

	AddListenerCmd.Flags().StringVarP(&subsystem_addlistener_trtype,
		"trtype", "t", "",
		"NVMe-oF transport type: e.g., tcp")
	AddListenerCmd.MarkFlagRequired("trtype")

	AddListenerCmd.Flags().StringVarP(&subsystem_addlistener_traddr,
		"traddr", "i", "",
		"NVMe-oF target address: e.g., an ip address")
	AddListenerCmd.MarkFlagRequired("traddr")

	AddListenerCmd.Flags().StringVarP(&subsystem_addlistener_trsvcid,
		"trsvcid", "p", "",
		"NVMe-oF transport service id: e.g., a port number")
	AddListenerCmd.MarkFlagRequired("trsvcid")

}
