package cmd

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"pnconnector/src/errors"
	"pnconnector/src/log"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"

	"github.com/spf13/cobra"
)

var FileCmd = &cobra.Command{
	Use:   "file [json file]",
	Short: "JSON file input for Poseidon OS",
	Long: `Execute json file for Poseidon OS.

Usage : 

Input JSON file
Single file is available.

You can set IPv4 address and the port number to Poseidon OS confiruing config.yaml file or flags.
Default values are as below:
	IP   : 127.0.0.1
	Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) < 1 {
			return errors.New("Please input JSON file to input")
		}

		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		FileInput(cmd, args)
	},
}

func init() {

}

func FileInput(cmd *cobra.Command, args []string) {

	b, err := ioutil.ReadFile(args[0])

	if err != nil {
		fmt.Println(err)
		return
	}

	request := model.Request{}

	err = json.Unmarshal(b, &request)

	if err != nil {
		fmt.Println("invalid json file :", err)
		return
	}

	log.Info(request)

	InitConnect()
	res, err := iBoFOS.SendRequestJson(request)

	if err != nil {
		fmt.Println(err)
	} else {
		PrintReqRes(request, res)
	}
}
