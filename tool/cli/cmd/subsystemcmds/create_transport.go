package subsystemcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
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

		param := messages.CreateTransportParam{
			TRANSPORTTYPE: transport_create_trtype,
			BUFCACHESIZE:  transport_create_bufcachesize,
			NUMSHAREDBUF:  transport_create_numsharedbuf,
		}

		uuid := globals.GenerateUUID()

		req := messages.BuildReqWithParam(command, uuid, param)

		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var transport_create_trtype = ""
var transport_create_bufcachesize = 0
var transport_create_numsharedbuf = 0

func init() {
	CreateTransportCmd.Flags().StringVarP(&transport_create_trtype,
		"trtype", "t", "",
		"Transport type (ex. TCP).")
	CreateTransportCmd.MarkFlagRequired("trtype")

	CreateTransportCmd.Flags().IntVarP(&transport_create_bufcachesize,
		"buf-cache-size", "c", 0,
		"The number of shared buffers to reserve for each poll group (default : 64).")
	CreateTransportCmd.Flags().IntVarP(&transport_create_numsharedbuf,
		"num-shared-buf", "", 0,
		"The number of pooled data buffers available to the transport.")
}
