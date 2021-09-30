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
	Long: `
Create an array for PoseidonOS. 

Syntax: 
	poseidonos-cli array create (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--data-devs | -d) DeviceNameList (--spare | -s) DeviceName [--raid RaidType] .

Example: 
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEARRAY"

		req := buildCreateArrayReq()
		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Error("error:", err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Build a CreateArrayReq using flag values from commandline and return it
func buildCreateArrayReq() messages.Request {

	// Split a string (comma separate) that contains comma-separated device names into strings
	// and add them to a string array.
	dataDevs := parseDeviceList(create_array_dataDevsList)
	spareDevs := parseDeviceList(create_array_spareDevsList)

	var buffer [1]messages.DeviceNameList
	buffer[0].DEVICENAME = create_array_buffer

	param := messages.CreateArrayParam{
		ARRAYNAME: create_array_arrayName,
		RAID:      create_array_raid,
		BUFFER:    buffer,
		DATA:      dataDevs,
		SPARE:     spareDevs,
	}

	uuid := globals.GenerateUUID()

	req := messages.Request{
		RID:     uuid,
		COMMAND: "CREATEARRAY",
		PARAM:   param,
	}

	return req
}

// Parse comma-separated device list string and return the device list
func parseDeviceList(devsList string) []messages.DeviceNameList {

	if devsList == "" {
		return nil
	}

	devsListSlice := strings.Split(devsList, ",")

	var devs []messages.DeviceNameList
	for _, str := range devsListSlice {
		var devNameList messages.DeviceNameList // Single device name that is splitted
		devNameList.DEVICENAME = str
		devs = append(devs, devNameList)
	}

	return devs
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var create_array_arrayName = ""
var create_array_raid = ""
var create_array_buffer = ""
var create_array_spareDevsList = ""
var create_array_dataDevsList = ""

func init() {
	CreateArrayCmd.Flags().StringVarP(&create_array_arrayName,
		"array-name", "a", "", "The name of the array to create.")
	CreateArrayCmd.MarkFlagRequired("array-name")

	CreateArrayCmd.Flags().StringVarP(&create_array_dataDevsList,
		"data-devs", "d", "",
		"A comma-separated list of devices to be used as the data devices.")
	CreateArrayCmd.MarkFlagRequired("data-devs")

	CreateArrayCmd.Flags().StringVarP(&create_array_spareDevsList,
		"spare", "s", "", "The name of device to be used as the spare.")

	CreateArrayCmd.Flags().StringVarP(&create_array_buffer,
		"buffer", "b", "", "The name of device to be used as the buffer.")
	CreateArrayCmd.MarkFlagRequired("buffer")

	CreateArrayCmd.Flags().StringVarP(&create_array_raid,
		"raid", "r", "RAID5",
		"The RAID type of the array to create. RAID5 is used when not specified.")
}
