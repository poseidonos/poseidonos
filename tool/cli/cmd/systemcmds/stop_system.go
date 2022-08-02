package systemcmds

import (
	"fmt"
	"os"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

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
	RunE: func(cmd *cobra.Command, args []string) error {

		if stop_system_isForced == false {
			conf := displaymgr.AskConfirmation(
				"WARNING: This may affect the I/O operations in progress!!!\n" +
					"Do you really want to stop PoseidonOS?")
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "STOPSYSTEM"

		req, buildErr := buildStopSystemReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		reqJson, err := protojson.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		res, gRpcErr := grpcmgr.SendStopSystem(req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printErr := displaymgr.PrintProtoResponse(command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}

		return nil
	},
}

func buildStopSystemReq(command string) (*pb.StopSystemRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.StopSystemRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

var stop_system_isForced = false

func init() {
	StopSystemCmd.Flags().BoolVarP(&stop_system_isForced,
		"force", "", false,
		"Force to stop PoseidonOS.")
}
