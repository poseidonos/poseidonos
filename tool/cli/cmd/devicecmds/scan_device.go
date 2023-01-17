package devicecmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"fmt"
	"log"

	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

//TODO(mj): function for --detail flag needs to be implemented.
var ScanDeviceCmd = &cobra.Command{
	Use:   "scan",
	Short: "Scan devices in the system.",
	Long: `
Scan devices in the system. Use this command when PoseidonOS has 
(re)started.

Syntax:
	poseidonos-cli device scan
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SCANDEVICE"
		uuid := globals.GenerateUUID()

		req := &pb.ScanDeviceRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				posMgr, err := grpcmgr.GetPOSManager()
				if err != nil {
					log.Fatalf("failed to connect to POS: %v", err)
				}
				res, req, err := posMgr.ScanDevice()
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
				reqJson, err = protojson.MarshalOptions{
					EmitUnpopulated: true,
				}.Marshal(req)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf request: %v", err)
				}
			}

			displaymgr.PrintRequest(string(reqJson))
			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
