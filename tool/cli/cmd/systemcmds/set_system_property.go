package systemcmds

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

// TODO(mj): Currently, this command only supports REBUILDPERFIMPACT command.
// This command should be extended to support other commands to set the property of PoseidonOS as well.
var SetSystemPropCmd = &cobra.Command{
	Use:   "set-property",
	Short: "Set the property of PoseidonOS.",
	Long: `
Set the property of PoseidonOS.
(Note: this command is not officially supported yet.
 It might be  possible this command cause an error.)

Syntax:
	poseidonos-cli system set-property [--rebuild-impact (highest | medium | lowest)]

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact lowest
          `,
	Run: func(cmd *cobra.Command, args []string) {
		// TODO(mj): now the message format is for REBUILDPERFIMPACT only.
		// The message format should be extended for other properties also.

		var command = "REBUILDPERFIMPACT"
		uuid := globals.GenerateUUID()

		param := &pb.SetSystemPropertyRequest_Param{Level: set_system_property_level}
		req := &pb.SetSystemPropertyRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
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
				res, err := grpcmgr.SendSetSystemPropertyRpc(req)
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
var set_system_property_level = ""

func init() {
	SetSystemPropCmd.Flags().StringVarP(&set_system_property_level,
		"rebuild-impact", "", "",
		`The impact of rebuilding process on the I/O performance.
With high rebuilding-impact, the rebuilding process may
interfere with I/O operations more. Therefore, I/O operations may
slow down although rebuilding process becomes accelerated. 
`)
}
