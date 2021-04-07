package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	_ "pnconnector/src/setting"
	_ "encoding/json"
	"fmt"
	_ "github.com/c2h5oh/datasize"
	"github.com/google/uuid"
	"github.com/spf13/cobra"
	"reflect"
	"strings"
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
	{"access", "", ""},
	{"operation", "", ""},
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
}

var wbtCmd = &cobra.Command{
	Use:   "wbt [testname]",
	Short: "wbt for Poseidon OS",
	Long: `
Send WBT name and arguments to Poseidon OS and get a result fommated by JSON.

You can set ip and port number for connent to Poseidon OS using config.yaml or flags.
Default value is as below.

IP   : 127.0.0.1
Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) == 0 {
			return errors.New("wbt msg need one more argument!!!")
		}

		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		WBT(cmd, args)
	},
}

func init() {

	rootCmd.AddCommand(wbtCmd)

	for i, _ := range argList {
		wbtCmd.PersistentFlags().StringVarP(&argList[i].value, argList[i].name, argList[i].short, "", "set "+argList[i].name)
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
		PrintReqRes(req, res)
	}

	return res, err
}
