package arraycmds

import (
	"encoding/json"
	"fmt"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
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

		// TODO(mj): Currently, ARRAYLIST command sends ARRAYINFO command to the server
		// when an array is specified.
		// Those commands will be merged later.
		var (
			command = ""
			req     = messages.Request{}
		)

		if list_array_arrayName != "" {

			command = "ARRAYINFO"

			param := messages.ListArrayParam{
				ARRAYNAME: list_array_arrayName,
			}

			req = messages.BuildReqWithParam(command, globals.GenerateUUID(), param)

		} else {
			command = "LISTARRAY"

			req = messages.Request{
				RID:       globals.GenerateUUID(),
				COMMAND:   command,
				REQUESTOR: "cli",
			}

		}

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
