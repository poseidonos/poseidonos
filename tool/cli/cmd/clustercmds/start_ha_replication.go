package clustercmds

import (
	pb "cli/api"
	"fmt"
	"log"
	"time"

	"cli/cmd/globals"

	"cli/cmd/displaymgr"
	"cli/cmd/grpcmgr"

	_ "github.com/lib/pq"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var StartHaReplicationCmd = &cobra.Command{
	Use:   "start-rep",
	Short: "Start replication.",
	Long: `
Start replication.

Syntax:
	poseidonos-cli cluster start-rep flags
          `,
	Run: func(cmd *cobra.Command, args []string) {

		command := "STARTHAREPLICATION"
		uuid := globals.GenerateUUID()

		param := buildHaReplStartParam()
		req := &pb.StartHaReplicationRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			var resJSON string

			res, err := grpcmgr.SendStartHaReplication(req)
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

var (
	start_ha_replication_primary_node_name         = ""
	start_ha_replication_primary_array_name        = ""
	start_ha_replication_primary_volume_name       = ""
	start_ha_replication_primary_wal_volume_name   = ""
	start_ha_replication_secondary_node_name       = ""
	start_ha_replication_secondary_array_name      = ""
	start_ha_replication_secondary_volume_name     = ""
	start_ha_replication_secondary_wal_volume_name = ""
	start_ha_replication_timestamp                 = ""
)

func buildHaReplStartParam() *pb.StartHaReplicationRequest_Param {
	now := time.Now()
	timeStampFormat := "2006-01-02T15:04:05.999999"
	start_ha_replication_timestamp = now.Format(timeStampFormat)

	param := &pb.StartHaReplicationRequest_Param{
		PrimaryNodeName:        start_ha_replication_primary_node_name,
		PrimaryArrayName:       start_ha_replication_primary_array_name,
		PrimaryVolumeName:      start_ha_replication_primary_volume_name,
		PrimaryWalVolumeName:   start_ha_replication_primary_wal_volume_name,
		SecondaryNodeName:      start_ha_replication_secondary_node_name,
		SecondaryArrayName:     start_ha_replication_secondary_array_name,
		SecondaryVolumeName:    start_ha_replication_secondary_volume_name,
		SecondaryWalVolumeName: start_ha_replication_secondary_wal_volume_name,
		Timestamp:              start_ha_replication_timestamp,
	}

	return param
}

func init() {
	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_primary_node_name,
		"primary-node-name", "n", "", `Name of primary node.`)
	StartHaReplicationCmd.MarkFlagRequired("primary-node-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_primary_array_name,
		"primary-array-name", "a", "", `Name of array of primary node.`)
	StartHaReplicationCmd.MarkFlagRequired("primary-array-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_primary_volume_name,
		"primary-volume-name", "v", "", `Name of volume of primary node.`)
	StartHaReplicationCmd.MarkFlagRequired("primary-volume-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_primary_wal_volume_name,
		"primary-wal-volume-name", "w", "", `Name of wal volume of primary node.`)
	StartHaReplicationCmd.MarkFlagRequired("primary-wal-volume-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_secondary_node_name,
		"secondary-node-name", "m", "", `Name of secondary node.`)
	StartHaReplicationCmd.MarkFlagRequired("secondary-node-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_secondary_array_name,
		"secondary-array-name", "s", "", `Name of array of secondary node.`)
	StartHaReplicationCmd.MarkFlagRequired("secondary-array-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_secondary_volume_name,
		"secondary-volume-name", "b", "", `Name of volume of secondary node.`)
	StartHaReplicationCmd.MarkFlagRequired("secondary-volume-name")

	StartHaReplicationCmd.Flags().StringVarP(&start_ha_replication_secondary_wal_volume_name,
		"secondary-wal-volume-name", "e", "", `Name of wal volume of secondary node.`)
	StartHaReplicationCmd.MarkFlagRequired("secondary-wal-volume-name")
}
