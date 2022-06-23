package devicecmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"log"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var CreateDeviceCmd = &cobra.Command{
	Use:   "create",
	Short: "Create a buffer device.",
	Long: `
Create a buffer device.

Syntax:
	poseidonos-cli device create (--device-name | -d) DeviceName --num-blocks Number --block-size BlockSize --device-type uram --numa Number
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEDEVICE"
		uuid := globals.GenerateUUID()

		param := &pb.CreateDeviceRequest_Param{Name: create_device_deviceName,
			NumBlocks: create_device_numBlocks, BlockSize: create_device_blockSize,
			DevType: create_device_deviceType, Numa: create_device_numa}
		req := &pb.CreateDeviceRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendCreateDevice(req)
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var (
	create_device_deviceName        = ""
	create_device_numBlocks  uint32 = 8388608
	create_device_blockSize  uint32 = 512
	create_device_deviceType        = "uram"
	create_device_numa       uint32 = 0
)

func init() {
	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceName,
		"device-name", "d", "",
		"The name of the buffer device to create.")
	CreateDeviceCmd.MarkFlagRequired("device-name")

	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceType,
		"device-type", "t", "uram",
		"The type of the buffer device to create.")

	CreateDeviceCmd.Flags().Uint32VarP(&create_device_numBlocks,
		"num-blocks", "b", 8388608,
		"The number of blocks of the buffer device.")

	CreateDeviceCmd.Flags().Uint32VarP(&create_device_blockSize,
		"block-size", "s", 512,
		"The block size of the buffer device.")

	CreateDeviceCmd.Flags().Uint32VarP(&create_device_numa,
		"numa", "n", 0,
		"The NUMA node of the buffer device.")
}
