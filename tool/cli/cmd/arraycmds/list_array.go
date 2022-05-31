package arraycmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	"fmt"

	"github.com/labstack/gommon/log"
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
	Run: func(cmd *cobra.Command, args []string) {

		var command string

		if list_array_arrayName != "" {
			command = "ARRAYINFO"
			executeArrayInfoCmd(command)
		} else {
			command = "LISTARRAY"
			executeListArrayCmd(command)
		}
	},
}

func executeArrayInfoCmd(command string) {
	uuid := globals.GenerateUUID()
	param := &pb.ArrayInfoRequest_Param{Name: list_array_arrayName}
	req := &pb.ArrayInfoRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

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
			res, err := grpcmgr.SendArrayInfo(req)
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
}

func executeListArrayCmd(command string) {
	uuid := globals.GenerateUUID()
	req := &pb.ListArrayRequest{Command: command, Rid: uuid, Requestor: "cli"}

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
			res, err := grpcmgr.SendListArray(req)
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
