package telemetrycmds

import (
	"fmt"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	pb "kouros/api"

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

		reqParam, buildErr := buildSetTelemetryPropertyReqParam()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}
		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.SetTelemetryProperty(reqParam)

		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printErr := displaymgr.PrintProtoResponse(req.Command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}

		return nil
	},
}

func buildSetTelemetryPropertyReqParam() (*pb.SetTelemetryPropertyRequest_Param, error) {

	param := &pb.SetTelemetryPropertyRequest_Param{PublicationListPath: set_telemetry_property_publicationListPath}

	return param, nil
}

var set_telemetry_property_publicationListPath = ""

func init() {
	SetTelemetryPropCmd.Flags().StringVarP(&set_telemetry_property_publicationListPath,
		"publication-list-path", "", "", "The path of the telemetry publication list file.")
}
