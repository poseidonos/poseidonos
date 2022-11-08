package telemetrycmds

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

var StartTelemetryCmd = &cobra.Command{
	Use:   "start",
	Short: "Start the collection of telemetry data in PoseidonOS.",
	Long: `
Start the collection of telemetry data in PoseidonOS.

Syntax:
	poseidonos-cli telemetry start
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STARTTELEMETRY"
		uuid := globals.GenerateUUID()

		req := &pb.StartTelemetryRequest{Command: command, Rid: uuid, Requestor: "cli"}
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
				res, err := grpcmgr.SendStartTelemetryRpc(req)
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
