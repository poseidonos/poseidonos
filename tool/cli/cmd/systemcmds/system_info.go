package systemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/otelmgr"
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
		m := otelmgr.GetOtelManagerInstance()
		defer m.Shutdown()
		t := otelmgr.NewTracer()
		t.SetTrace(m.GetRootContext(), globals.SYSTEM_CMD_APP_NAME, globals.SYSTEM_INFO_FUNC_NAME)
		defer t.Release()

		var command = "SYSTEMINFO"

		req, buildErr := buildSystemInfoReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			t.RecordError(buildErr)
			return buildErr
		}

		reqJson, err := protojson.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			t.RecordError(err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		res, gRpcErr := grpcmgr.SendSystemInfo(t.GetContext(), req)
		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			t.RecordError(gRpcErr)
			return gRpcErr
		}

		printErr := displaymgr.PrintProtoResponse(command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			t.RecordError(printErr)
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
