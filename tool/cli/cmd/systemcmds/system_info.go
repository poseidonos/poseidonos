package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"fmt"
	"log"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/encoding/protojson"
)

var SystemInfoCmd = &cobra.Command{
	Use:   "info",
	Short: "Display information of PoseidonOS.",
	Long: `
Display the information of PoseidonOS.

Syntax:
	poseidonos-cli system info
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SYSTEMINFO"
		uuid := globals.GenerateUUID()

		req := &pb.SystemInfoRequest{Command: command, Rid: uuid, Requestor: "cli"}
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
				res, err := grpcmgr.SendSystemInfoRpc(req)
				if err != nil {
					status, _ := status.FromError(err)
					switch status.Code() {
					case codes.ResourceExhausted:
						fmt.Println("PoseidonOS may be processing a command. Please try after a while.")
						return
					}
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

func SendSystemInfoCmdSocket() {

}
