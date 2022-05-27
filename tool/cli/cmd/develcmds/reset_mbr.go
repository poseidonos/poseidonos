package develcmds

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

//TODO(mj): function for --detail flag needs to be implemented.
var ResetMBRCmd = &cobra.Command{
	Use:   "resetmbr",
	Short: "Reset MBR information of PoseidonOS.",
	Long: `
Reset MBR information of PoseidonOS (Previously: Array Reset command).
Use this command when you need to remove the all the arrays and 
reset the states of the devices. 

Syntax:
	poseidonos-cli devel resetmbr
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "RESETMBR"
		uuid := globals.GenerateUUID()

		req := &pb.ResetMbrRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJSON string

			if globals.EnableGrpc == false {
				resJSON = socketmgr.SendReqAndReceiveRes(string(reqJSON))
			} else {
				res, err := grpcmgr.SendResetMbrRpc(req)
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

func init() {

}
