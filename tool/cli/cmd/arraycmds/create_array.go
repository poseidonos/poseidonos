package arraycmds

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var CreateArrayCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create an array for PoseidonOS.",
	Long: `
Create an array for PoseidonOS. 

Syntax: 
	poseidonos-cli array create (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--data-devs | -d) DeviceNameList (--spare | -s) DeviceName [--raid RAID0 | RAID5 | RAID10 | RAID6] 
	[--no-raid]

Example 1 (creating an array with RAID5):  
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
	
Eample 2 (creating an array with RAID6): 	
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID6
`,

	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "CREATEARRAY"

		req, buildErr := buildCreateArrayReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		reqJson, err := protojson.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		res, gRpcErr := grpcmgr.SendCreateArray(req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printErr := displaymgr.PrintProtoResponse(command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}

		return nil
	},
}

// Build a CreateArrayReq using flag values from commandline and return it
func buildCreateArrayReq(command string) (*pb.CreateArrayRequest, error) {

	dataDevs := strings.Split(create_array_dataDevsList, ",")
	spareDevs := strings.Split(create_array_spareDevsList, ",")
	bufferDevs := strings.Split(create_array_bufferList, ",")

	if create_array_isNoRaid == true {
		if len(dataDevs) > maxNumDataDevsNoRaid {
			err := errors.New(`array with no RAID can have maximum ` +
				strconv.Itoa(maxNumDataDevsNoRaid) + ` data device(s).`)
			return nil, err
		}
		create_array_raid = "NONE"
	}

	if isRAIDConstMet(len(dataDevs), create_array_raid) == false {
		err := errors.New(`RAID10 only supports even number of data devices.`)
		return nil, err
	}

	param := &pb.CreateArrayRequest_Param{Name: create_array_arrayName, Raidtype: create_array_raid}

	if create_array_dataDevsList != "" {
		for _, dataDev := range dataDevs {
			param.Data = append(param.Data, &pb.DeviceNameList{DeviceName: dataDev})
		}
	}

	if create_array_spareDevsList != "" {
		for _, spare := range spareDevs {
			param.Spare = append(param.Spare, &pb.DeviceNameList{DeviceName: spare})
		}
	}

	if create_array_bufferList != "" {
		for _, buffer := range bufferDevs {
			param.Buffer = append(param.Buffer, &pb.DeviceNameList{DeviceName: buffer})
		}
	}

	uuid := globals.GenerateUUID()
	req := &pb.CreateArrayRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
