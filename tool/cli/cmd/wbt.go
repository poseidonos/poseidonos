package cmd

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"encoding/json"
	_ "encoding/json"
	"fmt"
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	_ "pnconnector/src/setting"
	"reflect"
	"strings"

	_ "github.com/c2h5oh/datasize"
	"github.com/google/uuid"
	"github.com/spf13/cobra"
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

	Run: func(cmd *cobra.Command, args []string) {
		WBT(cmd, args)
	},
}

func init() {

	for i, _ := range argList {
		WbtCmd.PersistentFlags().StringVarP(&argList[i].value, argList[i].name, argList[i].short, "", "set "+argList[i].name)
	}
}

func WBT(cmd *cobra.Command, args []string) (model.Response, error) {

	var req model.Request
	var res model.Response
	var err error

	var xrId string
	newUUID, err := uuid.NewUUID()

	if err == nil {
		xrId = newUUID.String()
	}

	InitConnect()

	if args[0] == "list_wbt" {
		req, res, err = iBoFOS.ListWBT(xrId, nil)
	} else {

		param := model.WBTParam{}
		param.TestName = args[0]

		for _, attr := range argList {
			if cmd.PersistentFlags().Changed(attr.name) {
				reflect.ValueOf(&param.Argv).Elem().FieldByName(strings.Title(strings.ToLower(attr.name))).SetString(attr.value)
			}
		}
		req, res, err = iBoFOS.WBT(xrId, param)
	}

	if err != nil {
		fmt.Println(err)
	} else {
		reqJSON, _ := json.Marshal(req)
		displaymgr.PrintRequest(string(reqJSON))

		resJSON, _ := json.Marshal(res)
		// mj: isDebug is true for WBT commands at default.
		// If globals.IsJSONRes is true, PrintResponse does not
		// display the debug output
		displaymgr.PrintResponse("WBT", string(resJSON), true, globals.IsJSONRes, globals.DisplayUnit)
	}

	return res, err
}
