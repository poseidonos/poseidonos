package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var AddSpareCmd = &cobra.Command{
	Use:   "addspare [flags]",
	Short: "Add a device as a spare to an array.",
	Long: `
Add a device as a spare to an array. Use this command when you want 
to add a spare device to an array that was created already. 
Please note that the capacity of the spare device must be equal to or greater than the smallest capacity among existing array devices.

Syntax:
	poseidonos-cli array addspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array addspare --spare nvme5 --array-name array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "ADDDEVICE"

		uuid := globals.GenerateUUID()

		param := &pb.AddSpareRequest_Param{Array: add_spare_arrayName}
		param.Spare = append(param.Spare, &pb.AddSpareRequest_SpareDeviceName{DeviceName: add_spare_spareDev})

		req := &pb.AddSpareRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendAddSpare(req)
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
var add_spare_arrayName = ""
var add_spare_spareDev = ""

func init() {
	AddSpareCmd.Flags().StringVarP(&add_spare_arrayName,
		"array-name", "a", "",
		"The name of the array to add a spare device.")
	AddSpareCmd.MarkFlagRequired("array-name")

	AddSpareCmd.Flags().StringVarP(&add_spare_spareDev,
		"spare", "s", "",
		"The name of the device to be added to the specified array.")
	AddSpareCmd.MarkFlagRequired("spare")
}
