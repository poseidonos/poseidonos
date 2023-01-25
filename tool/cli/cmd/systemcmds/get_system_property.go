package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

	pb "kouros/api"

	"github.com/spf13/cobra"
)

// TODO(mj): Currently, this command only supports REBUILDPERFIMPACT command.
// This command should be extended to support other commands to set the property of PoseidonOS as well.
var GetSystemPropCmd = &cobra.Command{
	Use:   "get-property",
	Short: "Display the property of PoseidonOS.",
	Long: `
Display the property (e.g., rebuilding performance impact) of PoseidonOS. 

Syntax:
	poseidonos-cli system get-property

Example:
	poseidonos-cli system get-property
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "GETSYSTEMPROPERTY"

		req, buildErr := buildGetSystemPropertyReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		printReqErr := displaymgr.PrintProtoReqJson(req)
		if printReqErr != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", printReqErr)
			return printReqErr
		}

		res, gRpcErr := grpcmgr.SendGetSystemProperty(req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printResErr := displaymgr.PrintProtoResponse(command, res)
		if printResErr != nil {
			fmt.Printf("failed to print the response: %v", printResErr)
			return printResErr
		}

		return nil
	},
}

func buildGetSystemPropertyReq(command string) (*pb.GetSystemPropertyRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.GetSystemPropertyRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func init() {
}
