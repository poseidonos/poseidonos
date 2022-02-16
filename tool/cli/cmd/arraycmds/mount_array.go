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

var MountArrayCmd = &cobra.Command{
	Use:   "mount [flags]",
	Short: "Mount an array to PoseidonOS.",
	Long: `
Mount an array to PoseidonOS. Use this command before creating a volume.
You can create a volume from an array only when the array is mounted. 

Syntax:
	mount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array mount --array-name Array0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "MOUNTARRAY"

		param := messages.MountArrayParam{
			ARRAYNAME: mount_array_arrayName,
			ENABLEWT:  mount_array_enableWriteThrough,
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_array_arrayName = ""
var mount_array_enableWriteThrough = false

func init() {
	MountArrayCmd.Flags().StringVarP(&mount_array_arrayName,
		"array-name", "a", "",
		"The name of the array to mount")
	MountArrayCmd.MarkFlagRequired("array-name")

	MountArrayCmd.Flags().BoolVarP(&mount_array_enableWriteThrough,
		"enable-write-through", "w", false,
		`When specified, the array to be mounted will work with write through mode.`)
}
