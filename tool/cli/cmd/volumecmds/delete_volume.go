package volumecmds

import (
	"fmt"
	"os"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var DeleteVolumeCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete a volume from PoseidonOS.",
	Long: `
Delete a volume from an array in PoseidonOS.

Syntax:
	poseidonos-cli volume delete (--volume-name | -v) VolumeName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli volume delete --volume-name Volume0 --array=name Array0
	
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var warningMsg = "WARNING: After deleting volume" + " " +
			delete_volume_volumeName + "," + " " +
			"you cannot recover the data of volume " +
			delete_volume_volumeName + " " +
			"in the array " +
			delete_volume_arrayName + "\n\n" +
			"Are you sure you want to delete volume" + " " +
			delete_volume_volumeName + "?"

		if delete_volume_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETEVOLUME"

		req, buildErr := buildDeleteVolumeReq(command)
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

		res, gRpcErr := grpcmgr.SendDeleteVolume(req)
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

func buildDeleteVolumeReq(command string) (*pb.DeleteVolumeRequest, error) {

	param := &pb.DeleteVolumeRequest_Param{
		Name:  delete_volume_volumeName,
		Array: delete_volume_arrayName,
	}

	uuid := globals.GenerateUUID()

	req := &pb.DeleteVolumeRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var delete_volume_volumeName = ""
var delete_volume_arrayName = ""
var delete_volume_isForced = false

func init() {
	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_volumeName,
		"volume-name", "v", "",
		"The Name of the volume to delete.")
	DeleteVolumeCmd.MarkFlagRequired("volume-name")

	DeleteVolumeCmd.Flags().StringVarP(&delete_volume_arrayName,
		"array-name", "a", "",
		"The Name of the array where the volume belongs to.")
	DeleteVolumeCmd.MarkFlagRequired("array-name")

	DeleteVolumeCmd.Flags().BoolVarP(&delete_volume_isForced,
		"force", "", false,
		"Force to delete the volume (volume must be unmounted first).")
}
