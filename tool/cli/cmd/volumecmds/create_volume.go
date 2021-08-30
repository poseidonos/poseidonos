package volumecmds

import (
	"encoding/json"
	"pnconnector/src/log"
	"strings"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"code.cloudfoundry.org/bytefmt"
	"github.com/spf13/cobra"
)

var CreateVolumeCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create a volume from an Array for PoseidonOS.",
	Long: `Create a volume from an Array for PoseidonOS.

Syntax: 
	poseidonos-cli volume create (--volume-name | -v) VolumeName (--array-name | -a) ArrayName --size VolumeSize [--maxiops" IOPS] [--maxbw Bandwidth] .
	
	- VolumeSize Unit: B, K, KB, G, GB, ... (binary units (base-2))
	(If the unit is not input, it will be B in default)

Example: 
	poseidonos-cli volume create --volume-name Volume0 --array-name volume0 --size 1024GB --maxiops 1000 --maxbw 100GB/s
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEVOLUME"

		createVolumeReq := formCreateVolumeReq()
		reqJSON, err := json.Marshal(createVolumeReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Debug("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func formCreateVolumeReq() messages.Request {

	volumeSize := strings.TrimSpace(create_volume_volumeSize)
	volumeSize = strings.ToUpper(create_volume_volumeSize)

	if volumeSize[len(volumeSize)-1:] != "B" {
		volumeSize += "B"
	}

	volumeSizeInByte, err := bytefmt.ToBytes(volumeSize)
	if err != nil {
		log.Fatal("error:", err)
	}

	createVolumeParam := messages.CreateVolumeParam{
		VOLUMENAME:   create_volume_volumeName,
		VOLUMESIZE:   volumeSizeInByte,
		MAXIOPS:      create_volume_maxIOPS,
		MAXBANDWIDTH: create_volume_maxBandwidth,
		ARRAYNAME:    create_volume_arrayName,
	}

	createVolumeReq := messages.Request{
		RID:     "fromfakeclient",
		COMMAND: "CREATEVOLUME",
		PARAM:   createVolumeParam,
	}

	return createVolumeReq
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var create_volume_volumeName = ""
var create_volume_arrayName = ""
var create_volume_volumeSize = ""
var create_volume_maxIOPS = 0
var create_volume_maxBandwidth = 0

func init() {
	CreateVolumeCmd.Flags().StringVarP(&create_volume_volumeName, "volume-name", "v", "", "Name of the volume to create")
	CreateVolumeCmd.MarkFlagRequired("volume-name")

	CreateVolumeCmd.Flags().StringVarP(&create_volume_arrayName, "array-name", "a", "", "Name of the array where the volume is created from")
	CreateVolumeCmd.MarkFlagRequired("array-name")

	CreateVolumeCmd.Flags().StringVarP(&create_volume_volumeSize, "size", "", "0", "The size of the volume. KB, MB, GB, TB, PB, and EB are supported")
	CreateVolumeCmd.MarkFlagRequired("size")

	CreateVolumeCmd.Flags().IntVarP(&create_volume_maxIOPS, "maxiops", "", 0, "The maximum IOPS for the volume in Kilo")
	CreateVolumeCmd.Flags().IntVarP(&create_volume_maxBandwidth, "maxbw", "", 0, "The maximum bandwidth for the volume in MB/s")
}
