package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var SystemInfoCmd = &cobra.Command{
	Use:   "info",
	Short: "Display information of PoseidonOS.",
	Long: `
Display the information of PoseidonOS.

Syntax:
	poseidonos-cli system info
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "SYSTEMINFO"

		req, buildErr := buildSystemInfoReq(command)
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

		res, gRpcErr := grpcmgr.SendSystemInfo(req)
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

func buildSystemInfoReq(command string) (*pb.SystemInfoRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.SystemInfoRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func init() {
}

func SendSystemInfoCmdSocket() {

}
