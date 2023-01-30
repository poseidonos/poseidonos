package qoscmds

import (
	"fmt"
	"strings"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var VolumeResetCmd = &cobra.Command{
	Use:   "reset [flags]",
	Short: "Reset QoS policy for a volume(s) of PoseidonOS.",
	Long: `
Reset QoS policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos reset (--volume-name | -v) VolumeName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli qos reset --volume-name Volume0 --array-name Array0
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		reqParam := formResetVolumePolicyReqParam()

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.ResetQoSVolumePolicy(reqParam)
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

func formResetVolumePolicyReqParam() *pb.QosResetVolumePolicyRequest_Param {

	volumeNameListSlice := strings.Split(volumeReset_volumeNameList, ",")
	var volumeNames []*pb.QosVolumeNameParam
	for _, str := range volumeNameListSlice {
		volumeName := &pb.QosVolumeNameParam{
			VolumeName: str,
		}
		volumeNames = append(volumeNames, volumeName)
	}
	param := &pb.QosResetVolumePolicyRequest_Param{
		Array: volumeReset_arrayName,
		Vol:   volumeNames,
	}

	return param
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var volumeReset_volumeNameList = ""
var volumeReset_arrayName = ""

func init() {
	VolumeResetCmd.Flags().StringVarP(&volumeReset_volumeNameList,
		"volume-name", "v", "",
		"A comma-seperated list of volumes to set qos policy for")
	VolumeResetCmd.MarkFlagRequired("volume-name")

	VolumeResetCmd.Flags().StringVarP(&volumeReset_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume is created from")
	VolumeResetCmd.MarkFlagRequired("array-name")
}
