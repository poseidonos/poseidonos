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

var SetTelemetryPropCmd = &cobra.Command{
	Use:   "set-property",
	Short: "Set properties of telemetry.",
	Long: `
  Set properties of telemetry.

Syntax:
  poseidonos-cli telemetry set-property --publication-list-path path
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "SETTELEMETRYPROPERTY"
		req, buildErr := buildSetTelemetryPropertyReq(command)
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

		res, gRpcErr := grpcmgr.SendSetTelemetryPropertyRpc(req)
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

func buildSetTelemetryPropertyReq(command string) (*pb.SetTelemetryPropertyRequest, error) {

	param := &pb.SetTelemetryPropertyRequest_Param{PublicationListPath: set_telemetry_property_publicationListPath}

	uuid := globals.GenerateUUID()
	req := &pb.SetTelemetryPropertyRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

var set_telemetry_property_publicationListPath = ""

func init() {
	SetTelemetryPropCmd.Flags().StringVarP(&set_telemetry_property_publicationListPath,
		"publication-list-path", "", "", "The path of the telemetry publication list file.")
}
