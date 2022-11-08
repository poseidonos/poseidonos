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

var ListHaReplicationCmd = &cobra.Command{
	Use:   "lr",
	Short: "List all replications in the cluster.",
	Long: `
List all replications in the cluster.

Syntax:
	poseidonos-cli cluster lr
          `,
	Run: func(cmd *cobra.Command, args []string) {

		command := "LISTHAREPLICATION"
		uuid := globals.GenerateUUID()

		req := &pb.ListHaReplicationRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJson))

		if !(globals.IsTestingReqBld) {
			var resJson string

			res, err := grpcmgr.SendListHaReplication(req)
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
