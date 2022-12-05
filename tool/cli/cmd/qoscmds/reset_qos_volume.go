package qoscmds

import (
    "fmt"
    "strings"

    pb "cli/api"
    "cli/cmd/displaymgr"
    "cli/cmd/globals"
    "cli/cmd/grpcmgr"

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

        var command = "RESETQOSVOLUMEPOLICY"

        req := formResetVolumePolicyReq(command)
        reqJson, err := protojson.MarshalOptions{
            EmitUnpopulated: true,
        }.Marshal(req)
        if err != nil {
            fmt.Printf("failed to marshal the protobuf request: %v", err)
            return err
        }
        displaymgr.PrintRequest(string(reqJson))
        res, gRpcErr := grpcmgr.SendQosResetVolumePolicy(req)
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

func formResetVolumePolicyReq(command string) *pb.QosResetVolumePolicyRequest {

    volumeNameListSlice := strings.Split(volumePolicy_volumeNameList, ",")
    var volumeNames []*pb.QosVolumeNameParam
    for _, str := range volumeNameListSlice {
        volumeName := &pb.QosVolumeNameParam{
            VolumeName: str,
        }
        volumeNames = append(volumeNames, volumeName)
    }
    uuid := globals.GenerateUUID()
    param := &pb.QosResetVolumePolicyRequest_Param{
        Array:   volumePolicy_arrayName,
        Vol:     volumeNames,
    }
    req := &pb.QosResetVolumePolicyRequest{
        Command:   command,
        Rid:       uuid,
        Requestor: "cli",
        Param:     param,
    }

    return req
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
