package systemcmds

import (
	"fmt"
	"os"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"

	"github.com/spf13/cobra"
)

var StopSystemCmd = &cobra.Command{
	Use:   "stop",
	Short: "Stop PoseidonOS.",
	Long: `
Stop PoseidonOS.

Syntax:
	poseidonos-cli system stop
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		if stop_system_isForced == false {
			confMsg := "WARNING: This may affect the I/O operations in progress!!!\n" +
				"Do you really want to stop PoseidonOS?"
			conf := displaymgr.AskConfirmation(confMsg)
			if conf == false {
				os.Exit(0)
			}
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.StopPoseidonOS()

		printReqErr := displaymgr.PrintProtoReqJson(req)
		if printReqErr != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", printReqErr)
			return printReqErr
		}

		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}

		printResErr := displaymgr.PrintProtoResponse(req.Command, res)
		if printResErr != nil {
			fmt.Printf("failed to print the response: %v", printResErr)
			return printResErr
		}

		return nil
	},
}

var stop_system_isForced = false

func init() {
	StopSystemCmd.Flags().BoolVarP(&stop_system_isForced,
		"force", "", false,
		"Force to stop PoseidonOS.")
}
