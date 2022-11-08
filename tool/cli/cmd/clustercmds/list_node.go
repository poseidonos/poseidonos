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
		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			var resJSON string

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
			resJSON = string(resByte)

			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

func init() {

}
