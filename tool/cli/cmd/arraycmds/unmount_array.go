package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	"os"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var UnmountArrayCmd = &cobra.Command{
	Use:   "unmount [flags]",
	Short: "Unmount an array from PoseidonOS.",
	Long: `
Unmount an array from PoseidonOS.

Syntax:
	unmount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array unmount --array-name Array0
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var warningMsg = "WARNING: After unmounting array" + " " +
			unmount_array_arrayName + "," + " " +
			"all the volumes in the array will be unmounted.\n" +
			"In addition, progressing I/Os may fail if any.\n\n" +
			"Are you sure you want to unmount array" + " " +
			unmount_array_arrayName + "?"

		if unmount_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "UNMOUNTARRAY"

		req, buildErr := buildUnmountArrayReq(command)
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

		res, gRpcErr := grpcmgr.SendUnmountArray(req)
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

func buildUnmountArrayReq(command string) (*pb.UnmountArrayRequest, error) {
	uuid := globals.GenerateUUID()

	param := &pb.UnmountArrayRequest_Param{Name: unmount_array_arrayName}
	req := &pb.UnmountArrayRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var unmount_array_arrayName = ""
var unmount_array_isForced = false

func init() {
	UnmountArrayCmd.Flags().StringVarP(&unmount_array_arrayName,
		"array-name", "a", "",
		"The name of the array to unmount.")
	UnmountArrayCmd.MarkFlagRequired("array-name")

	UnmountArrayCmd.Flags().BoolVarP(&unmount_array_isForced,
		"force", "", false,
		"Force to unmount this array.")
}
