package arraycmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var AutocreateArrayCmd = &cobra.Command{
	Use:   "autocreate [flags]",
	Short: "Automatically create an array for PoseidonOS.",
	Long: `
Automatically create an array for PoseidonOS with the number of 
devices the user specifies. Use this command when you do not care 
which devices are included to the array. This command will automatically
create an array with the devices in the same NUMA.

Syntax: 
	AutoCreateArrayCmd = "autocreate" ("--array-name" | "-a") ArrayName ("--num-buffer" | "-b") Number 
	("--num-data-devs" | "-d") Number [("--num-spare" | "-s") Number] ["--raid" RaidType] .

Example: 
	poseidonos-cli array autocreate --array-name Array0 --buffer device0 --num-data-devs 3 --num-spare 1
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "AUTOCREATEARRAY"

		req := buildAutocreateArrayReq()
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
func buildAutocreateArrayReq() messages.Request {

	// Assume that at most one device is used as a buffer.
	var buffer [1]messages.DeviceNameList
	buffer[0].DEVICENAME = autocreate_array_buffer

	param := messages.AutocreateArrayParam{
		ARRAYNAME:    autocreate_array_arrayName,
		RAID:         autocreate_array_raid,
		BUFFER:       buffer,
		NUMDATADEVS:  autocreate_array_dataDevs,
		NUMSPAREDEVS: autocreate_array_spare,
	}

	uuid := globals.GenerateUUID()

	req := messages.Request{
		RID:     uuid,
		COMMAND: "AUTOCREATEARRAY",
		PARAM:   param,
	}

	return req
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var autocreate_array_arrayName = ""
var autocreate_array_raid = ""
var autocreate_array_buffer = ""
var autocreate_array_spare = 0
var autocreate_array_dataDevs = 0

func init() {
	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_arrayName,
		"array-name", "a", "",
		"The name of the array to create.")
	AutocreateArrayCmd.MarkFlagRequired("array-name")

	AutocreateArrayCmd.Flags().IntVarP(&autocreate_array_dataDevs,
		"num-data-devs", "d", 0,
		`The number of of the data devices. POS will select the data
		devices in the same NUMA as possible.`)
	AutocreateArrayCmd.MarkFlagRequired("data-devs")

	AutocreateArrayCmd.Flags().IntVarP(&autocreate_array_spare,
		"num-spare", "s", 0,
		"Number of devices to be used as the spare.")

	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_buffer,
		"buffer", "b", "", "The name of device to be used as buffer.")
	AutocreateArrayCmd.MarkFlagRequired("buffer")

	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_raid,
		"raid", "r", "RAID5",
		"The RAID type of the array to create. RAID5 is used when not specified.")

}
