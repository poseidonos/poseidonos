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

var StopTelemetryCmd = &cobra.Command{
	Use:   "stop",
	Short: "Stop the collection of telemetry data in PoseidonOS.",
	Long: `
Stop the collection of telemetry data in PoseidonOS.

Syntax:
	poseidonos-cli telemetry stop
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STOPTELEMETRY"
		uuid := globals.GenerateUUID()

		req := &pb.StopTelemetryRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJson))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				res, err := grpcmgr.SendStopTelemetryRpc(req)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
			}

			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {
}
