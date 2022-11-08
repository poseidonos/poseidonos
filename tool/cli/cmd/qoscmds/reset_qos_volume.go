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

	Run: func(cmd *cobra.Command, args []string) {

		var command = "RESETQOSVOLUMEPOLICY"

		req := formVolumeResetReq(command)
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

func formVolumeResetReq(command string) messages.Request {

	volumeNameListSlice := strings.Split(volumeReset_volumeNameList, ",")
	var volumeNames []messages.VolumeNameList
	for _, str := range volumeNameListSlice {
		var volumeNameList messages.VolumeNameList // Single device name that is splitted
		volumeNameList.VOLUMENAME = str
		volumeNames = append(volumeNames, volumeNameList)
	}

	param := messages.VolumePolicyParam{
		VOLUMENAME: volumeNames,
		ARRAYNAME:  volumeReset_arrayName,
	}

	uuid := globals.GenerateUUID()

	req := messages.BuildReqWithParam(command, uuid, param)

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
