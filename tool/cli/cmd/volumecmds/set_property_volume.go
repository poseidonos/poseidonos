package volumecmds

import (
	"encoding/json"
	"pnconnector/src/log"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

var SetPropertyVolumeCmd = &cobra.Command{
	Use:   "set-property [flags]",
	Short: "Set property of a volume of PoseidonOS.",
	Long: `Set property of a volume of PoseidonOS.

Syntax: 
	poseidonos-cli volume set-property --volume-name VolumeName (--array-name | -a) ArrayName [--maxiops" IOPS] [--maxbw Bandwidth] .

Example: 
	poseidonos-cli volume set-property --volume-name Volume0 --array-name nolume0 --maxiops 500 --maxbw 50GB/s
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "UPDATEVOLUMEQOS"

		setPropertyVolumeReq := formSetPropertyVolumeReq()
		reqJSON, err := json.Marshal(setPropertyVolumeReq)
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

func formSetPropertyVolumeReq() messages.Request {

	setPropertyVolumeParam := messages.SetPropertyVolumeParam{
		VOLUMENAME:   setproperty_volume_volumeName,
		MAXIOPS:      setproperty_volume_maxIOPS,
		MAXBANDWIDTH: setproperty_volume_maxBandwidth,
		ARRAYNAME:    setproperty_volume_arrayName,
	}

	setPropertyVolumeReq := messages.Request{
		RID:     "fromCLI",
		COMMAND: "UPDATEVOLUMEQOS",
		PARAM:   setPropertyVolumeParam,
	}

	return setPropertyVolumeReq
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var setproperty_volume_volumeName = ""
var setproperty_volume_arrayName = ""
var setproperty_volume_maxIOPS = 0
var setproperty_volume_maxBandwidth = 0

func init() {
	SetPropertyVolumeCmd.Flags().StringVarP(&setproperty_volume_volumeName, "volume-name", "", "", "Name of the volume to create")
	SetPropertyVolumeCmd.Flags().StringVarP(&setproperty_volume_arrayName, "array-name", "a", "", "Name of the array where the volume is created from")
	SetPropertyVolumeCmd.Flags().IntVarP(&setproperty_volume_maxIOPS, "maxiops", "", 0, "The maximum IOPS for the volume")
	SetPropertyVolumeCmd.Flags().IntVarP(&setproperty_volume_maxBandwidth, "maxbw", "", 0, "The maximum bandwidth for the volume")
}
