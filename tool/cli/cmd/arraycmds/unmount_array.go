package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"os"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var UnmountArrayCmd = &cobra.Command{
	Use:   "unmount [flags]",
	Short: "Unmount an array from PoseidonOS.",
	Long: `
Unmount an array from PoseidonOS.

Syntax:
	unmount (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array unmount --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: After unmounting array" + " " +
			unmount_array_arrayName + "," + " " +
			"all the volumes in the array will be unmounted.\n" +
			"In addition, progressing I/Os may fail if any.\n\n" +
			"Are you sure you want to unmount array" + " " +
			unmount_array_arrayName + "?"

		if unmount_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "UNMOUNTARRAY"

		uuid := globals.GenerateUUID()

		param := &pb.UnmountArrayRequest_Param{Name: unmount_array_arrayName}
		req := &pb.UnmountArrayRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
				res, err := grpcmgr.SendUnmountArray(req)
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
var unmount_array_arrayName = ""
var unmount_array_isForced = false

func init() {
	UnmountArrayCmd.Flags().StringVarP(&unmount_array_arrayName,
		"array-name", "a", "",
		"The name of the array to unmount.")
	UnmountArrayCmd.MarkFlagRequired("array-name")

	UnmountArrayCmd.Flags().BoolVarP(&unmount_array_isForced,
		"force", "", false,
		"Force to unmount this array.")
}
