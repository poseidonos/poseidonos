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

var ListHaVolumeCmd = &cobra.Command{
	Use:   "lv",
	Short: "List all volumes in the cluster.",
	Long: `
List all nodes in the cluster.

Syntax:
	poseidonos-cli cluster lv
          `,
	Run: func(cmd *cobra.Command, args []string) {

		command := "LISTHAVOLUME"
		uuid := globals.GenerateUUID()

		req := &pb.ListHaVolumeRequest{Command: command, Rid: uuid, Requestor: "cli"}
		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			var resJSON string

			res, err := grpcmgr.SendListHaVolume(req)
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
