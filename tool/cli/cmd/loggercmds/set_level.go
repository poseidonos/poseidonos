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

var SetLevelCmd = &cobra.Command{
	Use:   "set-level",
	Short: "Set the filtering level of logger.",
	Long: `
Set the filtering level of logger.

Syntax:
	poseidonos-cli logger set-level --level [debug | info | warning | error | critical]
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SETLOGLEVEL"
		uuid := globals.GenerateUUID()

		param := &pb.SetLogLevelRequest_Param{Level: set_level_loggerLevel}
		req := &pb.SetLogLevelRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendSetLogLevel(req)
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

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var set_level_loggerLevel = ""

func init() {
	SetLevelCmd.Flags().StringVarP(&set_level_loggerLevel,
		"level", "", "",
		`The level of logger to set.
	
- critical: events that make the system not available.
- error: events when PoseidonOS cannot process the request
	because of an internal problem. 
- warning: events when unexpected user input has been detected. 
- debug: logs when the system is working properly.
- info: logs for debug binary.`)
	SetLevelCmd.MarkFlagRequired("level")
}
