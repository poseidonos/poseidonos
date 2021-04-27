package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"github.com/spf13/cobra"
)

var InternalCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
    "stop_rebuilding":  iBoFOS.StopRebuilding,
}

var internalCmd = &cobra.Command{
	Use:   "internal [msg]",
	Short: "Request for internal msg to Poseidon OS",
	Long: `Request for internal msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]            : [description]                                                    : [example of flag]

internal   : stop_rebuilding  : Stop rebuilding of an array                                      : --name ArrayName

If you want to input multiple flag parameter, you have to seperate with ",". 
For example, "-d dev1,dev2,dev3". seperation by space is not allowed.


You can set ip and port number for connect to Poseidon OS using config.yaml or flags.
Default value is as below.

IP   : 127.0.0.1
Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) != 1 {
			return errors.New("need an one msg !!!")
		}

		_, exist := InternalCommand[args[0]]

		if !exist {
			return errors.New("not available msg !!!")
		}

		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		Send(cmd, args)
	},
}

func init() {

	rootCmd.AddCommand(internalCmd)
    internalCmd.PersistentFlags().UintVar(&prio, "prio", 0, "set prio [0/1/2]")
    internalCmd.PersistentFlags().UintVar(&weight, "weight", 0, "set weight [0/1/2]")
	internalCmd.PersistentFlags().StringVarP(&name, "name", "n", "", "set name")
}
