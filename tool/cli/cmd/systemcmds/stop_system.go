package systemcmds

import (
	"fmt"
	"os"

	pb "kouros/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"github.com/spf13/cobra"
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
			confMsg := "WARNING: This may affect the I/O operations in progress!!!\n" +
				"Do you really want to stop PoseidonOS?"
			conf := displaymgr.AskConfirmation(confMsg)
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

		printReqErr := displaymgr.PrintProtoReqJson(req)
		if printReqErr != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", printReqErr)
			return printReqErr
		}

		res, gRpcErr := grpcmgr.SendStopSystem(req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printResErr := displaymgr.PrintProtoResponse(command, res)
		if printResErr != nil {
			fmt.Printf("failed to print the response: %v", printResErr)
			return printResErr
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
