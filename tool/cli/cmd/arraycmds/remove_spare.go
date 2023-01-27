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

var RemoveSpareCmd = &cobra.Command{
	Use:   "rmspare [flags]",
	Short: "Remove a spare device from an array of PoseidonOS.",
	Long: `
Remove a spare device from an array of PoseidonOS.

Syntax:
	poseidonos-cli array rmspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rmspare --spare DeviceName --array-name Array0
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		reqParam, buildErr := buildRemoveSpareReqParam()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.RemoveSpareDevice(reqParam)

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

func buildRemoveSpareReqParam() (*pb.RemoveSpareRequest_Param, error) {

	param := &pb.RemoveSpareRequest_Param{Array: remove_spare_arrayName}
	param.Spare = append(param.Spare, &pb.RemoveSpareRequest_SpareDeviceName{DeviceName: remove_spare_spareDevName})

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var remove_spare_spareDevName = ""
var remove_spare_arrayName = ""

func init() {
	RemoveSpareCmd.Flags().StringVarP(&remove_spare_arrayName, "array-name",
		"a", "",
		"The name of the array to remove the specified spare device.")
	RemoveSpareCmd.MarkFlagRequired("array-name")

	RemoveSpareCmd.Flags().StringVarP(&remove_spare_spareDevName,
		"spare", "s", "",
		"The name of the device to remove from the array.")
	RemoveSpareCmd.MarkFlagRequired("spare")
}
