package systemcmds

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

var StopSystemCmd = &cobra.Command{
	Use:   "stop",
	Short: "Stop PoseidonOS.",
	Long: `
Stop PoseidonOS.

Syntax:
	poseidonos-cli system stop
          `,
	Run: func(cmd *cobra.Command, args []string) {

		if stop_system_isForced == false {
			conf := displaymgr.AskConfirmation("WARNING: Stopping POS will " +
				"affect the progressing I/Os.\n\n" +
				"Are you sure you want to stop POS?")
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "STOPPOS"
		uuid := globals.GenerateUUID()

		req := &pb.SystemStopRequest{Command: command, Rid: uuid, Requestor: "cli"}

		if globals.EnableGrpc == false {
			reqJSON := protojson.Format(req)

			displaymgr.PrintRequest(string(reqJSON))

			// Do not send request to server and print response when testing request build.
			if !(globals.IsTestingReqBld) {
				resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
				displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
			}
		} else {
			if !(globals.IsTestingReqBld) {
				res, err := grpcmgr.SendSystemStopRpc(req)
				if err != nil {
					log.Fatalf("could not send request: %v", err)
				}
				displaymgr.PrintResponse(command, protojson.Format(res), globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
			}
		}
	},
}

var stop_system_isForced = false

func init() {
	StopSystemCmd.Flags().BoolVarP(&stop_system_isForced,
		"force", "", false,
		"Force to stop PoseidonOS.")
}
