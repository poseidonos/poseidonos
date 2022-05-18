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
			conf := displaymgr.AskConfirmation(
				"WARNING: This may affect the I/O operations in progress!!! " +
					"Do you really want to stop PoseidonOS?")
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "STOPPOS"
		uuid := globals.GenerateUUID()

		req := &pb.SystemStopRequest{Command: command, Rid: uuid, Requestor: "cli"}
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
				res, err := grpcmgr.SendSystemStopRpc(req)
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

var stop_system_isForced = false

func init() {
	StopSystemCmd.Flags().BoolVarP(&stop_system_isForced,
		"force", "", false,
		"Force to stop PoseidonOS.")
}
