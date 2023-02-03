package volumecmds

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

var UnmountVolumeCmd = &cobra.Command{
	Use:   "unmount [flags]",
	Short: "Unmount a volume to the host.",
	Long: `
Unmount a volume to the host.

Syntax:
	unmount (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 

Example: 
	poseidonos-cli volume unmount --volume-name Volume0 --array-name Volume0
	
         `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var warningMsg = "WARNING: After unmounting volume" + " " +
			unmount_volume_volumeName + " " +
			"in array " + unmount_volume_arrayName + "," + " " +
			"the progressing I/Os may fail if any.\n\n" +
			"Are you sure you want to unmount volume" + " " +
			unmount_volume_volumeName + "?"

		if unmount_volume_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		reqParam, buildErr := buildUnmountVolumeReqParam()
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.UnmountVolume(reqParam)

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

func buildUnmountVolumeReqParam() (*pb.UnmountVolumeRequest_Param, error) {

	param := &pb.UnmountVolumeRequest_Param{
		Name:  unmount_volume_volumeName,
		Array: unmount_volume_arrayName,
	}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var unmount_volume_volumeName = ""
var unmount_volume_arrayName = ""
var unmount_volume_isForced = false

func init() {
	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_volumeName,
		"volume-name", "v", "",
		"The name of the volume to unmount.")
	UnmountVolumeCmd.MarkFlagRequired("volume-name")

	UnmountVolumeCmd.Flags().StringVarP(&unmount_volume_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume belongs to.")
	UnmountVolumeCmd.MarkFlagRequired("array-name")

	UnmountVolumeCmd.Flags().BoolVarP(&unmount_volume_isForced,
		"force", "", false, "Force to unmount this volume.")
}
