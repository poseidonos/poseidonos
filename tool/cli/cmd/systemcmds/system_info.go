package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"log"

	pb "cli/api"

	"github.com/spf13/cobra"
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

		if globals.EnableGrpc == false {
			reqJSON := protojson.Format(req)

			displaymgr.PrintRequest(string(reqJSON))

			// Do not send request to server and print response when testing request build.
			if !(globals.IsTestingReqBld) {
				resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
				displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
			}
		} else {
			if !(globals.IsTestingReqBld) {
				res, err := grpcmgr.SendReqAndReceiveRes(req)
				if err != nil {
					log.Fatalf("could not send request: %v", err)
				}
				displaymgr.PrintResponse(command, protojson.Format(res), globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
			}
		}

	},
}

func init() {
}

func SendSystemInfoCmdSocket() {

}
