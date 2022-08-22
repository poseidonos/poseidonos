package volumecmds

import (
	"errors"
	"fmt"
	"strings"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"code.cloudfoundry.org/bytefmt"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var CreateVolumeCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create a volume from an array in PoseidonOS.",
	Long: `
Create a volume from an array in PoseidonOS.

Syntax: 
	poseidonos-cli volume create (--volume-name | -v) VolumeName 
	(--array-name | -a) ArrayName --size VolumeSize [--maxiops" IOPS] [--maxbw Bandwidth] [--iswalvol]

Example: 
	poseidonos-cli volume create --volume-name Volume0 --array-name volume0 
	--size 1024GB --maxiops 1000 --maxbw 100GB/s --iswalvol
`,

	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "CREATEVOLUME"

		req, buildErr := buildCreateVolumeReq(command)
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

		res, gRpcErr := grpcmgr.SendCreateVolume(req)
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

func buildCreateVolumeReq(command string) (*pb.CreateVolumeRequest, error) {

	if globals.IsValidVolName(create_volume_volumeName) == false {
		err := errors.New("The volume name must contain [a-zA-Z0-9_- ] only.")
		return nil, err
	}

	volumeSize := strings.ToUpper(strings.TrimSpace(create_volume_volumeSize))

	if strings.Contains(volumeSize, ".") == true {
		err := errors.New("The size of a volume must be an integer number.")
		return nil, err
	}

	if volumeSize[len(volumeSize)-1:] != "B" {
		volumeSize += "B"
	}

	volumeSizeInByte, err := bytefmt.ToBytes(volumeSize)
	if err != nil {
		return nil, err
	}

	param := &pb.CreateVolumeRequest_Param{
		Name:    create_volume_volumeName,
		Array:   create_volume_arrayName,
		Size:    volumeSizeInByte,
		Maxiops: create_volume_maxIOPS,
		Maxbw:   create_volume_maxBandwidth,
		Uuid:    create_volume_uuid,
	}

	uuid := globals.GenerateUUID()

	req := &pb.CreateVolumeRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var (
	create_volume_volumeName          = ""
	create_volume_arrayName           = ""
	create_volume_volumeSize          = ""
	create_volume_maxIOPS      uint64 = 0
	create_volume_maxBandwidth uint64 = 0
	create_volume_iswalvol            = false
	create_volume_uuid                = ""
)

func init() {
	CreateVolumeCmd.Flags().StringVarP(&create_volume_volumeName,
		"volume-name", "v", "",
		"The name of the volume to create.")
	CreateVolumeCmd.MarkFlagRequired("volume-name")

	CreateVolumeCmd.Flags().StringVarP(&create_volume_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume is created from.")
	CreateVolumeCmd.MarkFlagRequired("array-name")

	CreateVolumeCmd.Flags().StringVarP(&create_volume_volumeSize,
		"size", "", "0",
		`The size of the volume in B, K, KB, G, GB, ... (binary units (base-2))
If you do not specify the unit, it will be B in default. (Note: the size must be an integer number.)`)
	CreateVolumeCmd.MarkFlagRequired("size")

	CreateVolumeCmd.Flags().Uint64VarP(&create_volume_maxIOPS,
		"maxiops", "", 0,
		"The maximum IOPS for the volume in Kilo.")
	CreateVolumeCmd.Flags().Uint64VarP(&create_volume_maxBandwidth,
		"maxbw", "", 0,
		"The maximum bandwidth for the volume in MB/s.")

	CreateVolumeCmd.Flags().BoolVarP(&create_volume_iswalvol,
		"iswalvol", "", false,
		"If specified, the volume to be created will be a wal volume for HA.")

	CreateVolumeCmd.Flags().StringVarP(&create_volume_uuid,
		"uuid", "", "",
		"UUID for the volume to be created.")
}
