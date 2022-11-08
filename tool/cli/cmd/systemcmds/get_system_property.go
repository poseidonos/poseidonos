package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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

		reqJson, err := protojson.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		res, gRpcErr := grpcmgr.SendGetSystemProperty(req)
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

func buildGetSystemPropertyReq(command string) (*pb.GetSystemPropertyRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.GetSystemPropertyRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func init() {
}
