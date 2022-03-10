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
	Long: `
Create qos policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos create (--volume-name | -v) VolumeName
	(--array-name | -a) ArrayName [--maxiops" IOPS] [--maxbw Bandwidth]

Example: 
	poseidonos-cli qos create --volume-name vol1 --array-name Array0 --maxiops 500 --maxbw 100

NOTE:
    Current design of Qos supports only 1 Volume per Subsystem.
	If throttling values are set for more than one volume in a single subsystem,
	the throttling will be take effect only for the first mounted volume in the subsystem.
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEQOSVOLUMEPOLICY"

		req := formVolumePolicyReq(command)
		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func formVolumePolicyReq(command string) messages.Request {

	volumeNameListSlice := strings.Split(volumePolicy_volumeNameList, ",")
	var volumeNames []messages.VolumeNameList
	for _, str := range volumeNameListSlice {
		var volumeNameList messages.VolumeNameList // Single device name that is splitted
		volumeNameList.VOLUMENAME = str
		volumeNames = append(volumeNames, volumeNameList)
	}
	param := messages.VolumePolicyParam{
		VOLUMENAME:   volumeNames,
		MINIOPS:      volumePolicy_minIOPS,
		MAXIOPS:      volumePolicy_maxIOPS,
		MINBANDWIDTH: volumePolicy_minBandwidth,
		MAXBANDWIDTH: volumePolicy_maxBandwidth,
		ARRAYNAME:    volumePolicy_arrayName,
	}

	uuid := globals.GenerateUUID()

	req := messages.BuildReqWithParam(command, uuid, param)

	return req
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var volumePolicy_volumeNameList = ""
var volumePolicy_arrayName = ""

// -1 is the default value for the parameters. It means that the user has not set // any value for that parameter. This is done so that user doesnt need to pass all// parameter value for cli commands. Keeping 0 as the default value doesnt work,
// as the value comes as 0 even when user doesnt pass the parameter. When POS cli // server receives the value as -1 for a paramater, it doesnt process that
// parameter further.

var volumePolicy_minIOPS = -1
var volumePolicy_maxIOPS = -1
var volumePolicy_minBandwidth = -1
var volumePolicy_maxBandwidth = -1

func init() {
	VolumePolicyCmd.Flags().StringVarP(&volumePolicy_volumeNameList,
		"volume-name", "v", "",
		"A comma-seperated list of volumes to set qos policy for.")
	VolumePolicyCmd.MarkFlagRequired("volume-name")

	VolumePolicyCmd.Flags().StringVarP(&volumePolicy_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume is created from.")
	VolumePolicyCmd.MarkFlagRequired("array-name")

	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_minIOPS,
		"miniops", "", -1,
		"The minimum IOPS for the volume in KIOPS.")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_maxIOPS,
		"maxiops", "", -1,
		"The maximum IOPS for the volume in KIOPS.")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_minBandwidth,
		"minbw", "", -1,
		"The minimum bandwidth for the volume in MiB/s.")
	VolumePolicyCmd.Flags().IntVarP(&volumePolicy_maxBandwidth,
		"maxbw", "", -1,
		"The maximum bandwidth for the volume in MiB/s.")
}
