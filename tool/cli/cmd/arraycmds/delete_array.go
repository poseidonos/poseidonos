package arraycmds

import (
	"os"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var DeleteArrayCmd = &cobra.Command{
	Use:   "delete [flags]",
	Short: "Delete an array from PoseidonOS.",
	Long: `
Delete an array from PoseidonOS. After executing this command, 
the data and volumes in the array will be deleted too.

Syntax:
	poseidonos-cli array delete (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array delete --array-name Array0	
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = "WARNING: You are deleting array" + " " +
			delete_array_arrayName + "," + " " +
			"you will not be able to recover the data and the volumes in the array.\n" +
			"Are you sure you want to delete array" + " " +
			delete_array_arrayName + "?"

		if delete_array_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DELETEARRAY"

		uuid := globals.GenerateUUID()

		param := &pb.DeleteArrayRequest_Param{Name: delete_array_arrayName}
		req := &pb.DeleteArrayRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
				res, err := grpcmgr.SendDeleteArray(req)
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
var delete_array_arrayName = ""
var delete_array_isForced = false

func init() {
	DeleteArrayCmd.Flags().StringVarP(&delete_array_arrayName,
		"array-name", "a", "", "The name of the array to delete")
	DeleteArrayCmd.MarkFlagRequired("array-name")

	DeleteArrayCmd.Flags().BoolVarP(&delete_array_isForced,
		"force", "", false,
		"Force to delete this array (array must be unmounted first).")
}
