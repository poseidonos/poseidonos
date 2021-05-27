package arraycmds

import (
	"encoding/json"
	"strings"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var CreateArrayCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create an array for PoseidonOS.",
	Long: `Create an array for PoseidonOS.

Syntax: 
	poseidonos-cli array create (--array-name | -a) ArrayName --buffer DeviceName 
	--data-devs DeviceNameList --spare DeviceName [--raid RaidType] .

Example: 
	poseidonos-cli array create --array-name Array0 --buffer device0 --data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEARRAY"

		req := buildCreateArrayReq()
		reqJSON, err := json.Marshal(req)
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

// Build a CreateArrayReq using flag values from commandline and return it
func buildCreateArrayReq() messages.Request {

	// Split a string (comma separate) that contains comma-separated device names into strings
	// and add them to a string array.
	dataDevsListSlice := strings.Split(create_array_dataDevsList, ",")

	var dataDevs []messages.DeviceNameList
	for _, str := range dataDevsListSlice {
		var devNameList messages.DeviceNameList // Single device name that is splitted
		devNameList.DEVICENAME = str
		dataDevs = append(dataDevs, devNameList)
	}

	// Assume that at most one device is used as a buffer.
	var buffer [1]messages.DeviceNameList
	buffer[0].DEVICENAME = create_array_buffer

	createArrayParam := messages.CreateArrayParam{
		ARRAYNAME: create_array_arrayName,
		RAID:      create_array_raid,
		BUFFER:    buffer,
		DATA:      dataDevs,
	}

	req := messages.Request{
		RID:     "fromfakeclient",
		COMMAND: "CREATEARRAY",
		PARAM:   createArrayParam,
	}

	return req
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var create_array_arrayName = ""
var create_array_raid = ""
var create_array_buffer = ""
var create_array_spare = ""
var create_array_dataDevsList = ""

func init() {
	CreateArrayCmd.Flags().StringVarP(&create_array_arrayName, "array-name", "a", "", "Name of the array to create.")
	CreateArrayCmd.Flags().StringVarP(&create_array_raid, "raid", "", "", "RAID Type of the array to create.")
	CreateArrayCmd.Flags().StringVarP(&create_array_buffer, "buffer", "", "", "Name of device to be used as the buffer.")
	CreateArrayCmd.Flags().StringVarP(&create_array_spare, "spare", "", "", "Name of device to be used as the spare.")
	CreateArrayCmd.Flags().StringVarP(&create_array_dataDevsList, "data-devs", "", "", "A comma-separated names of devices to be used as the data devices.")
}
