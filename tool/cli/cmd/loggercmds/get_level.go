package loggercmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"fmt"
	"log"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var GetLevelCmd = &cobra.Command{
	Use:   "get-level",
	Short: "Get the filtering level of logger.",
	Long: `
Get the filtering level of logger.

Syntax:
	poseidonos-cli logger get-level

          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "GETLOGLEVEL"
		uuid := globals.GenerateUUID()

		req := &pb.GetLogLevelRequest{Command: command, Rid: uuid, Requestor: "cli"}
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
				res, err := grpcmgr.SendGetLogLevel(req)
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

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
