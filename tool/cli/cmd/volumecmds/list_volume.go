package volumecmds

import (
	pb "kouros/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

//TODO(mj): function for --detail flag needs to be implemented.
var ListVolumeCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List volumes of an array or display information of a volume.",
	Long: `
List volumes of an array or display information of a volume. Please note that the 'Remaining' field in the output 
shows an internal state that represents how many block addresses of the given volume are yet to be mapped. It
should not be interpreted as the file system free space.

Syntax:
	poseidonos-cli volume list (--array-name | -a) ArrayName [(--volume-name | -v) VolumeName]

Example1 (listing volumes of an array):
	poseidonos-cli volume list --array-name Array0

Example2 (displaying a detailed information of a volume):
	poseidonos-cli volume list --array-name Array0 --volume-name Volume0
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var (
			command string
			err     error
		)

		if list_volume_volumeName != "" {
			command = "VOLUMEINFO"
			err = executeVolumeInfoCmd(command)
		} else {
			command = "LISTVOLUME"
			err = executeListVolumeCmd(command)
		}

		return err
	},
}
func executeVolumeInfoCmd(command string) error {
    req, buildErr := buildVolumeInfoReq(command)
    if buildErr != nil {
        fmt.Printf("failed to build request: %v", buildErr)
        return buildErr
    }

    reqJson, err := protojson.MarshalOptions{
        EmitUnpopulated: true,
    }.Marshal(req)
    if err != nil {
        fmt.Printf("failed to marshal the protobuf request: %v", err)
        return err
    }
    displaymgr.PrintRequest(string(reqJson))
    fmt.Println("req",req)
    res, gRpcErr := grpcmgr.SendVolumeInfo(req)
    fmt.Println("res",res)
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
}

func executeListVolumeCmd(command string) error {
	req, buildErr := buildListVolumeReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendListVolume(req)
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
}

func buildListVolumeReq(command string) (*pb.ListVolumeRequest, error) {
	uuid := globals.GenerateUUID()
	param := &pb.ListVolumeRequest_Param{Array: list_volume_arrayName}
	req := &pb.ListVolumeRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
	return req, nil
}

func buildVolumeInfoReq(command string) (*pb.VolumeInfoRequest, error) {
    uuid := globals.GenerateUUID()
    param := &pb.VolumeInfoRequest_Param{Array: list_volume_arrayName, Volume:list_volume_volumeName}
    req := &pb.VolumeInfoRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
    return req, nil
}


// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var list_volume_arrayName = ""
var list_volume_volumeName = ""

func init() {
	ListVolumeCmd.Flags().StringVarP(&list_volume_arrayName,
		"array-name", "a", "",
		"The name of the array of volumes to list")
	ListVolumeCmd.MarkFlagRequired("array-name")

	ListVolumeCmd.Flags().StringVarP(&list_volume_volumeName,
		"volume-name", "v", "",
		"The name of the volume of the array to list."+"\n"+
			`When this is specified, the detailed information
		of this volume will be displayed.`)
}

func PrintResponse(response string) {
	fmt.Println(response)
}
