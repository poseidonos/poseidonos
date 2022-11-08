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
var UpdateEventWrrCmd = &cobra.Command{
	Use:   "update-event-wrr",
	Short: "Set the weights for backend events such as Flush, Rebuild, and GC.",
	Long: `
Set the weights for backend events such as Flush, Rebuild, and GC.

Syntax:
	poseidonos-cli devel update-event-wrr --name ( flush | fe_rebuild | rebuild | gc ) --weight ( 1 | 2 | 3 )
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "UPDATEEVENTWRRPOLICY"
		uuid := globals.GenerateUUID()

		param := &pb.UpdateEventWrrRequest_Param{Name: update_event_wrr_name, Weight: update_event_wrr_weight}
		req := &pb.UpdateEventWrrRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendUpdatEventWrr(req)
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

var (
	default_weight          int64  = 20
	update_event_wrr_name   string = ""
	update_event_wrr_weight int64  = default_weight
)

func init() {
	UpdateEventWrrCmd.Flags().StringVarP(&update_event_wrr_name,
		"name", "", "", "Event name.")
	UpdateEventWrrCmd.MarkFlagRequired("name")

	UpdateEventWrrCmd.Flags().Int64VarP(&update_event_wrr_weight,
		"weight", "", default_weight, "Weight.")
	UpdateEventWrrCmd.MarkFlagRequired("weight")
}
