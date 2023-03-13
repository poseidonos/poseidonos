package transportcmds

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

var ListTransportCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List NVMf transport to PoseidonOS.",
	Long: `
List NVMf transport to PoseidonOS.

Syntax:
	poseidonos-cli transport list

Example:
	poseidonos-cli transport list
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "LISTTRANSPORT"
		uuid := globals.GenerateUUID()

		req := &pb.ListTransportRequest{Command: command, Rid: uuid, Requestor: "cli"}

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
				res, req, err := posMgr.ListTransport()
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

func init() {
}
