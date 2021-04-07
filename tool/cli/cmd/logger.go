package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"github.com/spf13/cobra"
)

var LoggerCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"info":         iBoFOS.LoggerInfo,
	"set_level":    iBoFOS.SetLogLevel,
	"get_level":    iBoFOS.GetLogLevel,
	"apply_filter": iBoFOS.ApplyLogFilter,
}

var loggerCmd = &cobra.Command{
	Use:   "logger [msg]",
	Short: "Request for logger msg to Poseidon OS",
	Long: `Request for logger msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]         : [description]                                                    : [example of flag]

logger     : info          : get current ibofos logger settings                               : not needed
logger     : set_level     : Set filtering level to logger.                                   : --level [log level]
           : get_level     : Get filtering level to logger.                                   : not needed
           : apply_filter  : Apply filtering policy to logger.                                : not needed


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

		_, exist := LoggerCommand[args[0]]

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

	rootCmd.AddCommand(loggerCmd)

	loggerCmd.PersistentFlags().StringVarP(&level, "level", "l", "", "set level")
}
