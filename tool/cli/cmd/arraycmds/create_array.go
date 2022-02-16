package arraycmds

import (
	"encoding/json"
	"errors"
	"strconv"
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
	(--data-devs | -d) DeviceNameList (--spare | -s) DeviceName [--raid RAID0 | RAID5 | RAID10] 
	[--no-raid]

Example: 
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEARRAY"

		req, err := buildCreateArrayReq(command)
		if err != nil {
			log.Error(err)
			return
		}

		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error(err)
			return
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			socketmgr.Connect()

			resJSON, err := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			if err != nil {
				log.Error(err)
				return
			}

			socketmgr.Close()

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Build a CreateArrayReq using flag values from commandline and return it
func buildCreateArrayReq(command string) (messages.Request, error) {

	req := messages.Request{}

	// Split a string (comma separate) that contains comma-separated device names into strings
	// and add them to a string array.
	dataDevs := parseDevList(create_array_dataDevsList)
	spareDevs := parseDevList(create_array_spareDevsList)
	bufferDevs := parseDevList(create_array_bufferList)

	if create_array_isNoRaid == true {
		if len(dataDevs) > maxNumDataDevsNoRaid {
			err := errors.New(`array with no RAID can have maximum ` +
				strconv.Itoa(maxNumDataDevsNoRaid) + ` data device(s).`)
			return req, err
		}
		create_array_raid = "NONE"
	}

	param := messages.CreateArrayParam{
		ARRAYNAME: create_array_arrayName,
		RAID:      strings.ToUpper(create_array_raid),
		BUFFER:    bufferDevs,
		DATA:      dataDevs,
		SPARE:     spareDevs,
	}

	uuid := globals.GenerateUUID()

	req = messages.BuildReqWithParam(command, uuid, param)

	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var (
	create_array_arrayName     = ""
	create_array_raid          = ""
	create_array_bufferList    = ""
	create_array_spareDevsList = ""
	create_array_dataDevsList  = ""
	create_array_isNoRaid      = false
)

func init() {
	CreateArrayCmd.Flags().StringVarP(&create_array_arrayName,
		"array-name", "a", "", "The name of the array to create.")
	CreateArrayCmd.MarkFlagRequired("array-name")

	CreateArrayCmd.Flags().StringVarP(&create_array_dataDevsList,
		"data-devs", "d", "",
		`A comma-separated list of devices to be used as the data devices.
When the capacities of the data devices are different, the total capacity
of this array will be truncated based on the smallest one.`)
	CreateArrayCmd.MarkFlagRequired("data-devs")

	CreateArrayCmd.Flags().StringVarP(&create_array_spareDevsList,
		"spare", "s", "", "The name of device to be used as the spare.")

	CreateArrayCmd.Flags().StringVarP(&create_array_bufferList,
		"buffer", "b", "", "The name of device to be used as the buffer.")
	CreateArrayCmd.MarkFlagRequired("buffer")

	CreateArrayCmd.Flags().BoolVarP(&create_array_isNoRaid,
		"no-raid", "n", false,
		`When specified, no RAID will be applied to this array (--raid flag will be ignored).`+
			`Array with no RAID can have maximum `+strconv.Itoa(maxNumDataDevsNoRaid)+` data device(s).`)

	CreateArrayCmd.Flags().StringVarP(&create_array_raid,
		"raid", "r", "RAID5",
		"The RAID type of the array to create. RAID5 is used when not specified.")

}
