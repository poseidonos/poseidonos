package qoscmds

import (
	"encoding/json"
    "strings"
    "fmt"
	"pnconnector/src/log"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

var ListQosCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List qos policy for a volume(s) of PoseidonOS.",
	Long: `List qos policy for a volume of PoseidonOS.

Syntax: 
	poseidonos-cli qos list [--volume-name VolumeName] [(--array-name | -a) ArrayName] .

Example: 
	poseidonos-cli qos create --volume-name Volume0 --array-name Array0
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "QOSLISTPOLICIES"

		qosListReq := formListQosReq()
		reqJSON, err := json.Marshal(qosListReq)
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

func formListQosReq() messages.Request {

    volumeNameListSlice := strings.Split(listQos_volumeNameList, ",")
    var volumeNames []messages.VolumeNameList
    for _, str := range volumeNameListSlice {
        var volumeNameList messages.VolumeNameList // Single device name that is splitted
        volumeNameList.VOLUMENAME = str
        volumeNames = append(volumeNames, volumeNameList)
    }

	listQosParam := messages.VolumePolicyParam{
		VOLUMENAME:   volumeNames,
		ARRAYNAME:    listQos_arrayName,
	}

	qosListReq := messages.Request{
		RID:     "fromCLI",
		COMMAND: "QOSLISTPOLICIES",
		PARAM:   listQosParam,
	}

	return qosListReq
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var listQos_volumeNameList = ""
var listQos_arrayName = ""

func init() {
	ListQosCmd.Flags().StringVarP(&listQos_volumeNameList, "volume-name", "", "", "A comma-seperated names of volumes to set qos policy for")
	ListQosCmd.Flags().StringVarP(&listQos_arrayName, "array-name", "a", "", "Name of the array where the volume is created from")
}

func PrintResponse(response string) {
    fmt.Println(response)
}
