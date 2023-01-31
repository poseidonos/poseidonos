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

var ListQosCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List QoS policy for a volume(s) of PoseidonOS.",
	Long: `
List QoS policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos list [(--volume-name | -v) VolumeName] [(--array-name | -a) ArrayName]

Example: 
	poseidonos-cli qos create --volume-name Volume0 --array-name Array0
          `,

	RunE: func(cmd *cobra.Command, args []string) error {

		reqParam, buildErr := buildListQOSPolicyReqParam()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}
		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.ListQoSVolumePolicy(reqParam)
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

func buildListQOSPolicyReqParam() (*pb.ListQOSPolicyRequest_Param, error) {
	volumeNameListSlice := strings.Split(listQos_volumeNameList, ",")
	var volumeNames []*pb.ListQOSPolicyRequest_Param_Volume
	for _, str := range volumeNameListSlice {
		var volumeNameList pb.ListQOSPolicyRequest_Param_Volume // Single device name that is splitted
		volumeNameList.VolumeName = str
		volumeNames = append(volumeNames, &volumeNameList)
	}

	param := &pb.ListQOSPolicyRequest_Param{Array: listQos_arrayName, Vol: volumeNames}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var listQos_volumeNameList = ""
var listQos_arrayName = ""

func init() {
	ListQosCmd.Flags().StringVarP(&listQos_volumeNameList,
		"volume-name", "v", "",
		"A comma-seperated list of volumes to set qos policy for.")
	ListQosCmd.MarkFlagRequired("volume-name")

	ListQosCmd.Flags().StringVarP(&listQos_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume is created from.")
	ListQosCmd.MarkFlagRequired("array-name")
}

func PrintResponse(response string) {
	fmt.Println(response)
}
