package qoscmds

import (
	"encoding/json"
	"pnconnector/src/log"
	"strings"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

var VolumePolicyCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create qos policy for a volume(s) of PoseidonOS.",
	Long: `Create qos policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos create (--volume-name | -v) VolumeName (--array-name | -a) ArrayName [--maxiops" IOPS] [--maxbw Bandwidth] .

Example: 
	poseidonos-cli qos create --volume-name Volume0 --array-name Array0 --maxiops 500 --maxbw 50GB/s
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "QOSCREATEVOLUMEPOLICY"

		volumePolicyReq := formVolumePolicyReq()
		reqJSON, err := json.Marshal(volumePolicyReq)
		if err != nil {
			log.Debug("error:", err)
		}
		displaymgr.PrintRequest(string(reqJSON))

		socketmgr.Connect()
		resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
		socketmgr.Close()
		displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
	},
}

func formVolumePolicyReq() messages.Request {

	volumeNameListSlice := strings.Split(volumePolicy_volumeNameList, ",")
	var volumeNames []messages.VolumeNameList
	for _, str := range volumeNameListSlice {
		var volumeNameList messages.VolumeNameList // Single device name that is splitted
		volumeNameList.VOLUMENAME = str
		volumeNames = append(volumeNames, volumeNameList)
	}
	if 0 == volumePolicy_minIOPS {
		volumePolicy_minIOPS = 0xFFFFFFFF
	}
	if 0 == volumePolicy_maxIOPS {
		volumePolicy_maxIOPS = 0xFFFFFFFF
	}
	if 0 == volumePolicy_minBandwidth {
		volumePolicy_minBandwidth = 0xFFFFFFFF
	}
	if 0 == volumePolicy_maxBandwidth {
		volumePolicy_maxBandwidth = 0xFFFFFFFF
	}
	volumePolicyParam := messages.VolumePolicyParam{
		VOLUMENAME:   volumeNames,
		MINIOPS:      volumePolicy_minIOPS,
		MAXIOPS:      volumePolicy_maxIOPS,
		MINBANDWIDTH: volumePolicy_minBandwidth,
		MAXBANDWIDTH: volumePolicy_maxBandwidth,
		ARRAYNAME:    volumePolicy_arrayName,
	}

	volumePolicyReq := messages.Request{
		RID:     "fromCLI",
		COMMAND: "QOSCREATEVOLUMEPOLICY",
		PARAM:   volumePolicyParam,
	}

	return volumePolicyReq
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var volumePolicy_volumeNameList = ""
var volumePolicy_arrayName = ""
var volumePolicy_minIOPS = -1
var volumePolicy_maxIOPS = -1
var volumePolicy_minBandwidth = -1
var volumePolicy_maxBandwidth = -1

func init() {
	VolumePolicyCmd.Flags().StringVarP(&volumePolicy_volumeNameList, "volume-name", "v", "", "A comma-seperated names of volumes to set qos policy for")
	VolumePolicyCmd.Flags().StringVarP(&volumePolicy_arrayName, "array-name", "a", "", "Name of the array where the volume is created from")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_minIOPS, "miniops", "", -1, "The minimum IOPS for the volume")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_maxIOPS, "maxiops", "", -1, "The maximum IOPS for the volume")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_minBandwidth, "minbw", "", -1, "The minimum bandwidth for the volume")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_maxBandwidth, "maxbw", "", -1, "The maximum bandwidth for the volume")
}
