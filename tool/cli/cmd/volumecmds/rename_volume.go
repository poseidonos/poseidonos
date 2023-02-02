package volumecmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

//TODO(mj): function for --detail flag needs to be implemented.
var RenameVolumeCmd = &cobra.Command{
	Use:   "rename [flags]",
	Short: "Rename a volume of PoseidonOS.",
	Long: `
Rename a volume of PoseidonOS.

Syntax:
	poseidonos-cli volume rename (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 
	(--new-volume-name | -n) VolumeName

Example (renaming a volume): 
	poseidonos-cli volume rename --volume-name OldVolumeName --array-name Array0 --new-volume-name NewVolumeName
          `,
	RunE: func(cmd *cobra.Command, args []string) error {
		reqParam, buildErr := buildRenameVolumeReqParam()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.VolumeRename(reqParam)

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

func buildRenameVolumeReqParam() (*pb.VolumeRenameRequest_Param, error) {

	param := &pb.VolumeRenameRequest_Param{
		Array:   rename_volume_arrayName,
		Name:    rename_volume_volumeName,
		Newname: rename_volume_newVolumeName,
	}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var rename_volume_arrayName = ""
var rename_volume_volumeName = ""
var rename_volume_newVolumeName = ""

func init() {
	RenameVolumeCmd.Flags().StringVarP(&rename_volume_arrayName,
		"array-name", "a", "",
		"The Name of the array of the volume to change.")
	RenameVolumeCmd.MarkFlagRequired("array-name")

	RenameVolumeCmd.Flags().StringVarP(&rename_volume_volumeName,
		"volume-name", "v", "",
		"The Name of the volume to change its name.")
	RenameVolumeCmd.MarkFlagRequired("volume-name")

	RenameVolumeCmd.Flags().StringVarP(&rename_volume_newVolumeName,
		"new-volume-name", "n", "",
		"The new name of the volume.")
	RenameVolumeCmd.MarkFlagRequired("new-volume-name")
}
