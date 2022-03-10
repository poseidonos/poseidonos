package devicecmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
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

		param := messages.CreateDeviceReqParam{
			DEVICENAME: create_device_deviceName,
			NUMBLOCKS:  create_device_numBlocks,
			BLOCKSIZE:  create_device_blockSize,
			DEVICETYPE: create_device_deviceType,
			NUMA:       create_device_numa,
		}

		uuid := globals.GenerateUUID()

		req := messages.BuildReqWithParam(command, uuid, param)

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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var create_device_deviceName = ""
var create_device_numBlocks = 8388608
var create_device_blockSize = 512
var create_device_deviceType = "uram"
var create_device_numa = 0

func init() {
	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceName,
		"device-name", "d", "",
		"The name of the buffer device to create.")
	CreateDeviceCmd.MarkFlagRequired("device-name")

	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceType,
		"device-type", "t", "uram",
		"The type of the buffer device to create.")

	CreateDeviceCmd.Flags().IntVarP(&create_device_numBlocks,
		"num-blocks", "b", 8388608,
		"The number of blocks of the buffer device.")

	CreateDeviceCmd.Flags().IntVarP(&create_device_blockSize,
		"block-size", "s", 512,
		"The block size of the buffer device.")

	CreateDeviceCmd.Flags().IntVarP(&create_device_numa,
		"numa", "n", 0,
		"The NUMA node of the buffer device.")
}
