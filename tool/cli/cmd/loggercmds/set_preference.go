package loggercmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var SetPrefCmd = &cobra.Command{
	Use:   "set-preference",
	Short: "Set the preferences (e.g., format) of logger.",
	Long: `
Set the preferences (e.g., format) of logger.

Syntax:
	poseidonos-cli logger set-preference [--json BooleanValue]
          `,
	Run: func(cmd *cobra.Command, args []string) {
		var command = "SETLOGPREFERENCE"
		uuid := globals.GenerateUUID()

		param := &pb.SetLogPreferenceRequest_Param{StructuredLogging: set_pref_req_strLogging}
		req := &pb.SetLogPreferenceRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendSetLogPreference(req)
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

var (
	set_pref_req_strLogging = ""
)

func init() {
	SetPrefCmd.Flags().StringVarP(&set_pref_req_strLogging,
		"structured-logging", "s", "false",
		`When specified as true, PoseidonOS will log the events in JSON form for structured logging.
Otherwise, the events will be logged in plain text form.`)
	SetPrefCmd.MarkFlagRequired("level")
}
