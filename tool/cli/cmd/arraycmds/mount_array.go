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

var MountArrayCmd = &cobra.Command{
	Use:   "mount [flags]",
	Short: "Mount an array to PoseidonOS.",
	Long: `
Mount an array to PoseidonOS. Use this command before creating a volume.
You can create a volume from an array only when the array is mounted. 

Syntax:
	mount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array mount --array-name Array0
	
         `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "MOUNTARRAY"

		reqParam, buildErr := buildMountArrayReqParam(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		if cmd.Flags().Changed("timeout") {
			posMgr = posMgr.WithTimeout(globals.ReqTimeout)
		}

		res, req, gRpcErr := posMgr.MountArray(reqParam)

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

func buildMountArrayReqParam(command string) (*pb.MountArrayRequest_Param, error) {

	param := &pb.MountArrayRequest_Param{Name: mount_array_arrayName,
		EnableWriteThrough: &mount_array_enableWriteThrough}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_array_arrayName = ""
var mount_array_enableWriteThrough = false
var mount_array_targetAddress = ""

func init() {
	MountArrayCmd.Flags().StringVarP(&mount_array_arrayName,
		"array-name", "a", "",
		"The name of the array to mount")
	MountArrayCmd.MarkFlagRequired("array-name")

	MountArrayCmd.Flags().BoolVarP(&mount_array_enableWriteThrough,
		"enable-write-through", "w", false,
		`When specified, the array to be mounted will work with write through mode.`)

	MountArrayCmd.Flags().StringVarP(&mount_array_targetAddress,
		"traddr", "i", "",
		`Default target IP address for the array.`)
}
