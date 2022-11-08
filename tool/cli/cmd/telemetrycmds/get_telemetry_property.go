package telemetrycmds

import (
	"fmt"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var GetTelemetryPropCmd = &cobra.Command{
	Use:   "get-property",
	Short: "Get properties of telemetry.",
	Long: `
  Get properties of telemetry.

Syntax:
  poseidonos-cli telemetry get-property
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "GETTELEMETRYPROPERTY"
		req, buildErr := buildGetTelemetryPropertyReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		reqJson, err := protojson.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		res, gRpcErr := grpcmgr.SendGetTelemetryProperty(req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printErr := displaymgr.PrintProtoResponse(command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}

		return nil
	},
}

func buildGetTelemetryPropertyReq(command string) (*pb.GetTelemetryPropertyRequest, error) {

	uuid := globals.GenerateUUID()
	req := &pb.GetTelemetryPropertyRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func init() {
}
