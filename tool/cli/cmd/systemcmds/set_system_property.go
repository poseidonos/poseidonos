package systemcmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

// TODO(mj): Currently, this command only supports REBUILDPERFIMPACT command.
// This command should be extended to support other commands to set the property of PoseidonOS as well.
var SetSystemPropCmd = &cobra.Command{
	Use:   "set-property",
	Short: "Set the property of PoseidonOS.",
	Long: `
Set the property of PoseidonOS.
(Note: this command is not officially supported yet.
 It might be  possible this command cause an error.)

Syntax:
	poseidonos-cli system set-property [--rebuild-impact (highest | medium | lowest)]

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact lowest
          `,
	RunE: func(cmd *cobra.Command, args []string) error {
		// TODO(mj): now the message format is for REBUILDPERFIMPACT only.
		// The message format should be extended for other properties also.

		var command = "REBUILDPERFIMPACT"
		req, buildErr := buildSetSystemPropertyReq(command)
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

		res, gRpcErr := grpcmgr.SendSetSystemProperty(req)
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

func buildSetSystemPropertyReq(command string) (*pb.SetSystemPropertyRequest, error) {
	uuid := globals.GenerateUUID()

	param := &pb.SetSystemPropertyRequest_Param{Level: set_system_property_level}
	req := &pb.SetSystemPropertyRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
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
