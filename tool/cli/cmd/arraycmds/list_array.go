package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

//TODO(mj): function for --detail flag needs to be implemented.
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

	req, buildErr := buildArrayInfoReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendArrayInfo(req)
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
	req, buildErr := buildListArrayReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendListArray(req)
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

func buildListArrayReq(command string) (*pb.ListArrayRequest, error) {
	uuid := globals.GenerateUUID()
	req := &pb.ListArrayRequest{Command: command, Rid: uuid, Requestor: "cli"}

	return req, nil
}

func buildArrayInfoReq(command string) (*pb.ArrayInfoRequest, error) {
	uuid := globals.GenerateUUID()
	param := &pb.ArrayInfoRequest_Param{Name: list_array_arrayName}
	req := &pb.ArrayInfoRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
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
