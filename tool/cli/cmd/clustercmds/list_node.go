package clustercmds

import (
	pb "cli/api"
	"fmt"
	"log"

	"cli/cmd/globals"

	"cli/cmd/displaymgr"
	"cli/cmd/grpcmgr"

	_ "github.com/lib/pq"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var ListNodeCmd = &cobra.Command{
	Use:   "ln",
	Short: "List all nodes in the system.",
	Long: `
List all nodes in the system.

Syntax:
	poseidonos-cli cluster ln
          `,
	Run: func(cmd *cobra.Command, args []string) {

		command := "LISTNODE"
		uuid := globals.GenerateUUID()

		req := &pb.ListNodeRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJson))

		if !(globals.IsTestingReqBld) {
			var resJson string

			res, err := grpcmgr.SendListNode(req)
			if err != nil {
				fmt.Println(err)
				log.Println(err)
				return
			}
			resByte, err := protojson.Marshal(res)
			if err != nil {
				log.Fatalf("failed to marshal the protobuf response: %v", err)
			}
			resJson = string(resByte)

			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {

}
