package arraycmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var RebuildArrayCmd = &cobra.Command{
	Use:   "rebuild [flags]",
	Short: "Instantly trigger the rebuild for an array.",
	Long: `
If there is a pending rebuild operation, immediately execute the rebuild.
Use this command when you would like to start array rebuild.

Syntax:
	rebuild (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rebuild --array-name Array0
	
         `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "REBUILDARRAY"

		reqParam, buildErr := buildRebuildArrayReqParam(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}

		res, req, gRpcErr := posMgr.RebuildArray(reqParam)

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

		printErr := displaymgr.PrintProtoResponse(command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}

		return nil
	},
}

func buildRebuildArrayReqParam(command string) (*pb.RebuildArrayRequest_Param, error) {

	param := &pb.RebuildArrayRequest_Param{Name: rebuild_array_arrayName}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var rebuild_array_arrayName = ""

func init() {
	RebuildArrayCmd.Flags().StringVarP(&rebuild_array_arrayName,
		"array-name", "a", "",
		"The name of the array to mount")
	RebuildArrayCmd.MarkFlagRequired("array-name")
}
