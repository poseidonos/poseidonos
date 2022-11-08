package subsystemcmds

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

var CreateTransportCmd = &cobra.Command{
	Use:   "create-transport [flags]",
	Short: "Create NVMf transport to PoseidonOS.",
	Long: `
Create NVMf transport to PoseidonOS.

Syntax:
	poseidonos-cli subsystem create-transport (--trtype | -t) TransportType [(--buf-cache-size | -c) BufCacheSize] [--num-shared-buf NumSharedBuffers]

Example:
	poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATETRANSPORT"
		uuid := globals.GenerateUUID()

		param := &pb.CreateTransportRequest_Param{
			TransportType: transport_create_trtype,
			BufCacheSize:  transport_create_bufcachesize,
			NumSharedBuf:  transport_create_numsharedbuf,
		}

		req := &pb.CreateTransportRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

		reqJSON, err := protojson.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		if !(globals.IsTestingReqBld) {
			var resJSON string

			if globals.EnableGrpc == false {
				resJSON = socketmgr.SendReqAndReceiveRes(string(reqJSON))
			} else {
				res, err := grpcmgr.SendCreateTransport(req)
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
var (
	transport_create_trtype             = ""
	transport_create_bufcachesize int32 = 0
	transport_create_numsharedbuf int32 = 0
)

func init() {
	CreateTransportCmd.Flags().StringVarP(&transport_create_trtype,
		"trtype", "t", "",
		"Transport type (ex. TCP).")
	CreateTransportCmd.MarkFlagRequired("trtype")

	CreateTransportCmd.Flags().Int32VarP(&transport_create_bufcachesize,
		"buf-cache-size", "c", 0,
		"The number of shared buffers to reserve for each poll group (default : 64).")
	CreateTransportCmd.Flags().Int32VarP(&transport_create_numsharedbuf,
		"num-shared-buf", "", 0,
		"The number of pooled data buffers available to the transport.")
}
