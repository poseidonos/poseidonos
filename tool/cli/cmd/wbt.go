package cmd

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	_ "encoding/json"
	"fmt"
	pb "kouros/api"
	"pnconnector/src/errors"
	_ "pnconnector/src/setting"
	"strings"

	_ "github.com/c2h5oh/datasize"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

type arg struct {
	name  string
	short string
	value string
}

var argList = []arg{
	{"name", "", ""},
	{"input", "i", ""},
	{"output", "o", ""},
	{"integrity", "", ""},
	{"filetype", "", ""},
	{"rba", "", ""},
	{"lba", "", ""},
	{"vsid", "", ""},
	{"lsid", "", ""},
	{"offset", "", ""},
	{"size", "", ""},
	{"count", "n", ""},
	{"pattern", "", ""},
	{"loc", "", ""},
	{"fd", "", ""},
	{"dev", "d", ""},
	{"normal", "", ""},
	{"urgent", "", ""},
	{"op", "", ""},
	{"cns", "", ""},
	{"nsid", "", ""},
	{"cdw10", "", ""},
	{"cdw11", "", ""},
	{"cdw12", "", ""},
	{"cdw13", "", ""},
	{"cdw14", "", ""},
	{"cdw15", "", ""},
	{"lbaf", "", ""},
	{"ms", "", ""},
	{"pi", "", ""},
	{"pil", "", ""},
	{"ses", "", ""},
	{"array", "", ""},
	{"volume", "", ""},
	{"module", "", ""},
	{"key", "", ""},
	{"value", "", ""},
	{"type", "", ""},
}

var WbtCmd = &cobra.Command{
	Use:   "wbt [testname]",
	Short: "White box test (WBT) commands for Poseidon OS",
	Long: `
Send WBT name and arguments to Poseidon OS and get a result fommated by JSON.

You can set IPv4 address and the port number to Poseidon OS confiruing config.yaml file or flags.
Default values are as below:
	IP   : 127.0.0.1
	Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) == 0 {
			return errors.New("WBT commands need at least one argument")
		}

		return nil
	},

	RunE: func(cmd *cobra.Command, args []string) error {
		return WBT(cmd, args)
	},
}

func init() {

	for i, _ := range argList {
		WbtCmd.PersistentFlags().StringVarP(&argList[i].value, argList[i].name, argList[i].short, "", "set "+argList[i].name)
	}
}

func WBT(cmd *cobra.Command, args []string) error {

	InitConnect()

	if args[0] == "list_wbt" {

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.ListWBT()
		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}
		printErr := displaymgr.PrintProtoResponse(req.Command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}
		return nil
	} else {
		argv := make(map[string]string)

		for _, attr := range argList {
			if cmd.PersistentFlags().Changed(attr.name) && attr.value != "" {
				argv[strings.ToLower(attr.name)] = attr.value
			}
		}

		param := &pb.WBTRequest_Param{
			Testname: args[0],
			Argv:     argv,
		}

		posMgr, err := grpcmgr.GetPOSManager()
		if err != nil {
			fmt.Printf("failed to connect to POS: %v", err)
			return err
		}
		res, req, gRpcErr := posMgr.WBT(param)

		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			fmt.Printf("failed to marshal the protobuf request: %v", err)
			return err
		}
		displaymgr.PrintRequest(string(reqJson))

		if gRpcErr != nil {
			globals.PrintErrMsg(gRpcErr)
			return gRpcErr
		}
		printErr := displaymgr.PrintWBTResponse(req.Command, res)
		if printErr != nil {
			fmt.Printf("failed to print the response: %v", printErr)
			return printErr
		}
		return nil
	}
	return nil
}
