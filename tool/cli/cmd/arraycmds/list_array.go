package arraycmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	pb "kouros/api"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

// TODO(mj): function for --detail flag needs to be implemented.
var ListArrayCmd = &cobra.Command{
	Use:   "list [flags]",
	Short: "List arrays of PoseidonOS or display information of an array.",
	Long: `
List arrays of PoseidonOS or display information of an array.
When you specify the name of a specific array, this command will
display the detailed information about the array. Otherwise, this
command will display the brief information about all the arrays
in PoseidonOS. 

Syntax:
	poseidonos-cli array list [(--array-name | -a) ArrayName]

Example 1 (listing all arrays): 
	poseidonos-cli array list

Example 2 (listing a specific array):
	poseidonos-cli array list --array-name Array0
          `,
	RunE: func(cmd *cobra.Command, args []string) error {

		var (
			command string
			err     error
		)

		if list_array_arrayName != "" {
			command = "ARRAYINFO"
			err = executeArrayInfoCmd(command)
		} else {
			command = "LISTARRAY"
			err = executeListArrayCmd(command)
		}

		return err
	},
}

func executeArrayInfoCmd(command string) error {

	reqParam, buildErr := buildArrayInfoReqParam(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	posMgr, err := grpcmgr.GetPOSManager()
	if err != nil {
		fmt.Printf("failed to connect to POS: %v", err)
		return err
	}
	res, req, gRpcErr := posMgr.ArrayInfo(reqParam)

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

	printErr := displaymgr.PrintProtoResponse(command, res)
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

func executeListArrayCmd(command string) error {

	posMgr, err := grpcmgr.GetPOSManager()
	if err != nil {
		fmt.Printf("failed to connect to POS: %v", err)
		return err
	}
	res, req, gRpcErr := posMgr.ListArray()

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

	printErr := displaymgr.PrintProtoResponse(command, res)
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

func buildArrayInfoReqParam(command string) (*pb.ArrayInfoRequest_Param, error) {
	param := &pb.ArrayInfoRequest_Param{Name: list_array_arrayName}

	return param, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var list_array_arrayName = ""

func init() {
	ListArrayCmd.Flags().StringVarP(&list_array_arrayName,
		"array-name", "a", "",
		`The name of the array to list. If not specified, all arrays
		will be displayed.`)
	//TODO(mj): function for --detail flag will be implemented
	//ListArrayCommand.Flags().BoolVarP(&showDetail, "detail", "d", false, "Show detail information of the array")
}

func PrintResponse(response string) {
	fmt.Println(response)
}
