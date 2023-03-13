package subsystemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	pb "kouros/api"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var RemoveListenerCmd = &cobra.Command{
	Use:   "remove-listener [flags]",
	Short: "Remove a listener to an NVMe-oF subsystem",
	Long: `
Remove a listener to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem remove-listener (--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId

Example:
	poseidonos-cli subsystem remove-listener -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.100.2.14 -p 1158

    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "REMOVELISTENER"
		uuid := globals.GenerateUUID()
		param := &pb.RemoveListenerRequest_Param{
			Subnqn:             subsystem_removelistener_subnqn,
			TransportType:      subsystem_removelistener_trtype,
			TargetAddress:      subsystem_removelistener_traddr,
			TransportServiceId: subsystem_removelistener_trsvcid,
		}

		req := &pb.RemoveListenerRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				posMgr, err := grpcmgr.GetPOSManager()
				if err != nil {
					log.Fatalf("failed to connect to POS: %v", err)
				}
				res, req, err := posMgr.RemoveListener(param)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
				reqJson, err = protojson.MarshalOptions{
					EmitUnpopulated: true,
				}.Marshal(req)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf request: %v", err)
				}
			}

			displaymgr.PrintRequest(string(reqJson))
			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var subsystem_removelistener_subnqn = ""
var subsystem_removelistener_trtype = ""
var subsystem_removelistener_traddr = ""
var subsystem_removelistener_trsvcid = ""

func init() {
	RemoveListenerCmd.Flags().StringVarP(&subsystem_removelistener_subnqn,
		"subnqn", "q", "",
		"The NQN of the subsystem to remove listener.")
	RemoveListenerCmd.MarkFlagRequired("subnqn")

	RemoveListenerCmd.Flags().StringVarP(&subsystem_removelistener_trtype,
		"trtype", "t", "",
		"NVMe-oF transport type: e.g., tcp")
	RemoveListenerCmd.MarkFlagRequired("trtype")

	RemoveListenerCmd.Flags().StringVarP(&subsystem_removelistener_traddr,
		"traddr", "i", "",
		"NVMe-oF target address: e.g., an ip address")
	RemoveListenerCmd.MarkFlagRequired("traddr")

	RemoveListenerCmd.Flags().StringVarP(&subsystem_removelistener_trsvcid,
		"trsvcid", "p", "",
		"NVMe-oF transport service id: e.g., a port number")
	RemoveListenerCmd.MarkFlagRequired("trsvcid")

}
