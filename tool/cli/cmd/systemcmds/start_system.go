package systemcmds

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"

	"github.com/labstack/gommon/log"
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
	Run: func(cmd *cobra.Command, args []string) {

		var command = "STARTPOS"

		uuid := globals.GenerateUUID()

		req := messages.BuildReq(command, uuid)

		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		if !(globals.IsJSONReq || globals.IsJSONRes) {
			fmt.Println("Launching PoseidonOS...")
		}

		displaymgr.PrintRequest(string(reqJSON))

		// TODO(mj): Here, we execute a script to run POS. This needs to be revised in the future.

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			// TODO(mj): Although go test for this command will be passed,
			// it will print out some error commands because of the file path
			// to the execution script. This needs to be fixed later.
			startScriptPath, _ := filepath.Abs(filepath.Dir(os.Args[0]))
			startScriptPath += "/../script/start_poseidonos.sh"
			err := exec.Command("/bin/sh", "-c", "sudo "+startScriptPath).Run()
			resJSON := ""
			uuid := globals.GenerateUUID()
			if err != nil {
				resJSON = `{"command":"STARTPOS","rid":"` + uuid + `"` + `,"result":{"status":{"code":11000,` +
					`"description":"PoseidonOS has failed to start with error code: 11000"}}}`
			} else {
				resJSON = `{"command":"STARTPOS","rid":"` + uuid + `","result":{"status":{"code":0,` +
					`"description":"Done! PoseidonOS has started!"}}}`
			}
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}

	},
}

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
