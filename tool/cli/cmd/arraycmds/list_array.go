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
	Short: "List arrays of PoseidonOS.",
	Long: `List arrays of PoseidonOS.

Syntax:
	poseidonos-cli array list [(--array-name | -a) ArrayName].

Example 1 (listing all arrays): 
	poseidonos-cli array list

Example 2 (listing a specific array):
	poseidonos-cli array list --array-name Array0
          `,
	Run: func(cmd *cobra.Command, args []string) {

		// TODO(mj): Currently, ARRAYLIST command sends ARRAYINFO command to the server
		// when an array is specified.
		// Those commands will be merged later.
		var command = ""
		var listArrayReq = messages.Request{}
		if list_array_arrayName == "" {
			command = "LISTARRAY"
			listArrayReq = messages.Request{
				RID:     "fromCLI",
				COMMAND: command,
			}
		} else {
			command = "ARRAYINFO"
			listArrayParam := messages.ListArrayParam{
				ARRAYNAME: list_array_arrayName,
			}

			listArrayReq = messages.Request{
				RID:     "fromCLI",
				COMMAND: command,
				PARAM:   listArrayParam,
			}
		}

		reqJSON, err := json.Marshal(listArrayReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		socketmgr.Connect()
		resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
		socketmgr.Close()

		displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var list_array_arrayName = ""

func init() {
	ListArrayCmd.Flags().StringVarP(&list_array_arrayName, "array-name", "a", "", "The Name of the array to list")
	//TODO(mj): function for --detail flag will be implemented
	//ListArrayCommand.Flags().BoolVarP(&showDetail, "detail", "d", false, "Show detail information of the array")
}

func PrintResponse(response string) {
	fmt.Println(response)
}
