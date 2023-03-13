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

var ListListenerCmd = &cobra.Command{
	Use:   "list-listener [flags]",
	Short: "List a listener to an NVMe-oF subsystem",
	Long: `
List a listener to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem list-listener (--subnqn | -q) SubsystemNQN

Example:
	poseidonos-cli subsystem list-listener -q nqn.2019-04.ibof:subsystem1

    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LISTLISTENER"
		uuid := globals.GenerateUUID()
		param := &pb.ListListenerRequest_Param{
			Subnqn:             subsystem_listlistener_subnqn,
		}

		req := &pb.ListListenerRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
				res, req, err := posMgr.ListListener(param)
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
var subsystem_listlistener_subnqn = ""

func init() {
	ListListenerCmd.Flags().StringVarP(&subsystem_listlistener_subnqn,
		"subnqn", "q", "",
		"The NQN of the subsystem to list listener.")
	ListListenerCmd.MarkFlagRequired("subnqn")

}
