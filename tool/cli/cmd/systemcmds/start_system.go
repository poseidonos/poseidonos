package systemcmds

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"

	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"

	"github.com/spf13/cobra"
)

var StartSystemCmd = &cobra.Command{
	Use:   "start",
	Short: "Start PoseidonOS.",
	Long: `
Start PoseidonOS.

Syntax:
	poseidonos-cli system start
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var command = "STARTPOS"
		req, buildErr := buildStartSystemReq(command)
		if buildErr != nil {
			fmt.Printf("failed to build request: %v", buildErr)
			return buildErr
		}

		printReqErr := displaymgr.PrintProtoReqJson(req)
		if printReqErr != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", printReqErr)
			return printReqErr
		}

		// Print human readable message only when JSON flags are not specified.
		if isNoJsonFlagSpecified() {
			fmt.Println("Launching PoseidonOS...")
		}

		// TODO(mj): Here, we execute a script to run POS instead of sending a request to server.
		// This must be changed because this only works in a local machine.
		startScriptPath, _ := filepath.Abs(filepath.Dir(os.Args[0]))
		startScriptPath += "/../script/start_poseidonos.sh"
		err := exec.Command("/bin/sh", "-c", "sudo "+startScriptPath).Run()
		resJson := ""
		uuid := globals.GenerateUUID()
		if err != nil {
			resJson = `{"command":"STARTPOS","rid":"` + uuid + `"` + `,"result":{"status":{"code":11000,` +
				`"description":"PoseidonOS has failed to start with error code: 11000"}}}`
		} else {
			resJson = `{"command":"STARTPOS","rid":"` + uuid + `","result":{"status":{"code":0,` +
				`"description":"Done! PoseidonOS has started!"}}}`
		}
		displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)

		return nil
	},
}

func buildStartSystemReq(command string) (*pb.StartSystemRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.StartSystemRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func isNoJsonFlagSpecified() bool {
	if globals.IsJSONReq || globals.IsJSONRes {
		return false
	}

	return true
}

func init() {

}
