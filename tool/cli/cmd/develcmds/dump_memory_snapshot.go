package develcmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"os"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

//TODO(mj): function for --detail flag needs to be implemented.
var DumpMemorySnapshotCmd = &cobra.Command{
	Use:   "dump-memory-snapshot",
	Short: "Dump a memory snapshot of running PoseidonOS.",
	Long: `
Dump a memory snapshot (core dump) of running PoseidonOS.
Use this command when you need to store the current memory snapshot of
PoseidonOS for debugging purpose. 

Syntax:
	poseidonos-cli devel dump-memory-snapshot --path FilePath
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var warningMsg = `WARNING: This command will create a very large size file.
Do you really want to proceed?`

		if dump_memory_snapshot_isForced == false {
			conf := displaymgr.AskConfirmation(warningMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		var command = "DUMPMEMORYSNAPSHOT"
		uuid := globals.GenerateUUID()

		param := &pb.DumpMemorySnapshotRequest_Param{Path: dump_memory_snapshot_filePath}
		req := &pb.DumpMemorySnapshotRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJson))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				res, err := grpcmgr.SendDumpMemorySnapshotRpc(req)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
			}

			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

var (
	dump_memory_snapshot_filePath = ""
	dump_memory_snapshot_isForced = false
)

func init() {
	DumpMemorySnapshotCmd.Flags().StringVarP(&dump_memory_snapshot_filePath,
		"path", "", "",
		"The path to store the snapshot")
	DumpMemorySnapshotCmd.MarkFlagRequired("path")

	DumpMemorySnapshotCmd.Flags().BoolVarP(&dump_memory_snapshot_isForced,
		"force", "", false, "Force to unmount this volume.")
}
