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

var SMARTLOGCmd = &cobra.Command{
	Use:   "smart-log",
	Short: "Display SMART log information of a device.",
	Long: `
Display SMART log information of a device.

Syntax:
	poseidonos-cli device smart-log (--device-name | -d) DeviceName
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SMARTLOG"
		uuid := globals.GenerateUUID()

		param := &pb.GetSmartLogRequest_Param{Name: smart_log_deviceName}
		req := &pb.GetSmartLogRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJson))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				res, err := grpcmgr.SendGetSmartLog(req)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
			}

			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var smart_log_deviceName = ""

func init() {
	SMARTLOGCmd.Flags().StringVarP(&smart_log_deviceName,
		"device-name", "d", "",
		"The name of the device to display the SMART log information.")
	SMARTLOGCmd.MarkFlagRequired("device-name")
}
