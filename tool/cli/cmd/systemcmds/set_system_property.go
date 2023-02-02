package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	pb "kouros/api"
	"strings"

	"github.com/spf13/cobra"
)

// TODO(mj): Currently, this command only supports REBUILDPERFIMPACT command.
// This command should be extended to support other commands to set the property of PoseidonOS as well.
var SetSystemPropCmd = &cobra.Command{
	Use:   "set-property",
	Short: "Set the property of PoseidonOS.",
	Long: `
Set the property of PoseidonOS.

Syntax:
	poseidonos-cli system set-property [--rebuild-impact (high | medium | low)]

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact low
          `,
	RunE: func(cmd *cobra.Command, args []string) error {
		// TODO(mj): now the message format is for REBUILDPERFIMPACT only.
		// The message format should be extended for other properties also.

		reqParam, buildErr := buildSetSystemPropertyReq()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.SetSystemProperty(reqParam)

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

func buildSetSystemPropertyReq() (*pb.SetSystemPropertyRequest_Param, error) {
	param := &pb.SetSystemPropertyRequest_Param{Level: strings.ToLower(set_system_property_level)}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var set_system_property_level = ""

func init() {
	SetSystemPropCmd.Flags().StringVarP(&set_system_property_level,
		"rebuild-impact", "", "",
		`The impact of rebuilding process on the I/O performance.
With high rebuilding-impact, the rebuilding process may
interfere with I/O operations more. Therefore, I/O operations may
slow down although rebuilding process becomes accelerated. 
`)
}
