package loggercmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var ApplyFilterCmd = &cobra.Command{
	Use:   "apply-filter",
	Short: "Apply a filtering policy to logger.",
	Long: `
Apply a filtering policy to logger.

  - Filtering file: when executing this command, PoseidonOS reads a filtering policy 
  stored in a file. You can set the file path of the filter in the PoseidonOS configuration 
  (the default path is /etc/conf/filter). If the file does not exist, you can create one.
  
  - Filter file format (EBNF):
  [ include: FilterList ]
  [ exclude: FIlterList ]
  FilterList = FilterNumber | FilterNumber,FilterList
  FilterNumber = { ( Number ) | ( Range ) }
  Number = { Digit }
  Range = Number-Number
  
  - Filter file example:
  include: 1002,1005,6230,2000-3000
  exclude: 1006,5003,8000-9000

Syntax:
  poseidonos-cli logger apply-filter
          `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "APPLYLOGFILTER"

		uuid := globals.GenerateUUID()

		applyFilterReq := messages.BuildReq(command, uuid)

		reqJSON, err := json.Marshal(applyFilterReq)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {

}
