package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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
	poseidonos-cli array autocreate (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--num-data-devs | -d) Number [(--num-spare | -s) Number] [--raid RaidType]
	[--no-raid] [--no-buffer]

Example: 
	poseidonos-cli array autocreate --array-name Array0 --buffer uram0 --num-data-devs 3 --num-spare 1
          `,

	Run: func(cmd *cobra.Command, args []string) {

		var command = "AUTOCREATEARRAY"

		req, err := buildAutoCreateArrayReq(command)
		if err != nil {
			fmt.Println("error: " + err.Error())
			return
		}

		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJSON string

			if globals.EnableGrpc == false {
				resJSON = socketmgr.SendReqAndReceiveRes(string(reqJSON))
			} else {
				res, err := grpcmgr.SendAutocreateArray(req)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJSON = string(resByte)
			}

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func buildAutoCreateArrayReq(command string) (*pb.AutocreateArrayRequest, error) {

	if autocreate_array_isNoRaid == true {
		if autocreate_array_numDataDevs > maxNumDataDevsNoRaid {
			err := errors.New(`array with no RAID can have maximum ` +
				strconv.Itoa(maxNumDataDevsNoRaid) + ` data device(s).`)
			return nil, err
		}
		autocreate_array_raid = "NONE"
	}

	if isRAIDConstMet(autocreate_array_numDataDevs, autocreate_array_raid) == false {
		err := errors.New(`RAID10 only supports even number of data devices.`)
		return nil, err
	}

	param := &pb.AutocreateArrayRequest_Param{Name: autocreate_array_arrayName,
		NumData: int32(autocreate_array_numDataDevs), NumSpare: int32(autocreate_array_numSpareDevs),
		Raidtype: autocreate_array_raid}

	bufferList := strings.Split(autocreate_array_bufferDevsList, ",")
	for _, buffer := range bufferList {
		param.Buffer = append(param.Buffer, &pb.DeviceNameList{DeviceName: buffer})
	}

	uuid := globals.GenerateUUID()
	req := &pb.AutocreateArrayRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var (
	autocreate_array_arrayName      = ""
	autocreate_array_raid           = ""
	autocreate_array_bufferDevsList = ""
	autocreate_array_numDataDevs    = 0
	autocreate_array_numSpareDevs   = 0
	autocreate_array_isNoRaid       = false
	autocreate_array_isNoBuffer     = false
)

func init() {
	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_arrayName,
		"array-name", "a", "",
		"The name of the array to create.")
	AutocreateArrayCmd.MarkFlagRequired("array-name")

	AutocreateArrayCmd.Flags().IntVarP(&autocreate_array_numDataDevs,
		"num-data-devs", "d", 0,
		`The number of of the data devices. POS will select the data
devices in the same NUMA as possible.`)
	AutocreateArrayCmd.MarkFlagRequired("data-devs")

	AutocreateArrayCmd.Flags().IntVarP(&autocreate_array_numSpareDevs,
		"num-spare", "s", 0,
		"Number of devices to be used as the spare.")

	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_bufferDevsList,
		"buffer", "b", "", "The name of device to be used as buffer.")
	AutocreateArrayCmd.MarkFlagRequired("buffer")

	AutocreateArrayCmd.Flags().StringVarP(&autocreate_array_raid,
		"raid", "r", "RAID5",
		"The RAID type of the array to create. RAID5 is used when not specified.")

	AutocreateArrayCmd.Flags().BoolVarP(&autocreate_array_isNoRaid,
		"no-raid", "n", false,
		`When specified, no RAID will be applied to this array (--raid flag will be ignored).`+
			`Array with no RAID can have maximum `+strconv.Itoa(maxNumDataDevsNoRaid)+` data device(s).`)
}
