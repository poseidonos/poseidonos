package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

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

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.GetSystemProperty()

		printReqErr := displaymgr.PrintProtoReqJson(req)
		if printReqErr != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", printReqErr)
			return printReqErr
		}

		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printResErr := displaymgr.PrintProtoResponse(req.Command, res)
		if printResErr != nil {
			fmt.Printf("failed to print the response: %v", printResErr)
			return printResErr
		}

		return nil
	},
}

func init() {
}
