package develcmds

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

//TODO(mj): function for --detail flag needs to be implemented.
var StopRebuildingCmd = &cobra.Command{
	Use:   "stop-rebuilding",
	Short: "Stop rebulding.",
	Long: `
Stop rebuilding.

Syntax:
	poseidonos-cli devel stop-rebuilding ( --array-name | -a ) ArrayName
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STOPREBUILDING"
		uuid := globals.GenerateUUID()

		param := &pb.StopRebuildingRequest_Param{Name: stop_rebuilding_arrayName}
		req := &pb.StopRebuildingRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendStopRebuildingRpc(req)
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

var stop_rebuilding_arrayName = ""

func init() {
	StopRebuildingCmd.Flags().StringVarP(&stop_rebuilding_arrayName,
		"array-name", "a", "", "Array name.")
	StopRebuildingCmd.MarkFlagRequired("array-name")
}
