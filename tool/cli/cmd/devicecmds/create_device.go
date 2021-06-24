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
	Long: `Create a buffer device.

Syntax:
	poseidonos-cli device create (--device-name | -d) DeviceName --num-blocks NumBlocks --block-size BlockSize --device-type ["uram"|"pram"].
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATEDEVICE"

		createDeviceReqParam := messages.CreateDeviceReqParam{
			DEVICENAME: create_device_deviceName,
			NUMBLOCKS:  create_device_numBlocks,
			BLOCKSIZE:  create_device_blockSize,
			DEVICETYPE: create_device_deviceType,
		}

		createDeviceReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   createDeviceReqParam,
		}

		reqJSON, err := json.Marshal(createDeviceReq)
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var create_device_deviceName = ""
var create_device_numBlocks = 512
var create_device_blockSize = 4096
var create_device_deviceType = ""

func init() {
	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceName, "device-name", "d", "", "The name of device to create")
	CreateDeviceCmd.Flags().StringVarP(&create_device_deviceType, "device-type", "", "", "The type of device to create")
	CreateDeviceCmd.Flags().IntVarP(&create_device_numBlocks, "num-blocks", "", 512, "The number of blocks in the device")
	CreateDeviceCmd.Flags().IntVarP(&create_device_blockSize, "block-size", "", 4096, "The block size of the device")

}
