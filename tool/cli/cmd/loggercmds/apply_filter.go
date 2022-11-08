package loggercmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"log"

	pb "cli/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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

		req := &pb.ApplyLogFilterRequest{Command: command, Rid: uuid, Requestor: "cli"}
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
				res, err := grpcmgr.SendApplyLogFilter(req)
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

func init() {

}
