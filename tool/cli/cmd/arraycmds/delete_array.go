package arraycmds

import (
	"fmt"
	"os"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var DeleteArrayCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete an array from PoseidonOS.",
	Long: `
Delete an array from PoseidonOS. After executing this command, 
the data and volumes in the array will be deleted too.

Syntax:
	poseidonos-cli array delete (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array delete --array-name Array0	
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var warningMsg = "WARNING: You are deleting array" + " " +
			delete_array_arrayName + "," + " " +
			"you will not be able to recover the data and the volumes in the array.\n" +
			"Are you sure you want to delete array" + " " +
			delete_array_arrayName + "?"

		if delete_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETEARRAY"
		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}

		reqParam, buildErr := buildDeleteArrayReqParam(command)

		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		res, req, gRpcErr := posMgr.DeleteArray(reqParam)
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

func buildDeleteArrayReqParam(command string) (*pb.DeleteArrayRequest_Param, error) {

	param := &pb.DeleteArrayRequest_Param{Name: delete_array_arrayName}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var delete_array_arrayName = ""
var delete_array_isForced = false

func init() {
	DeleteArrayCmd.Flags().StringVarP(&delete_array_arrayName,
		"array-name", "a", "", "The name of the array to delete")
	DeleteArrayCmd.MarkFlagRequired("array-name")

	DeleteArrayCmd.Flags().BoolVarP(&delete_array_isForced,
		"force", "", false,
		"Force to delete this array (array must be unmounted first).")
}
