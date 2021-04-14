package cmd

import (
	//    "pnconnector/src/log"
	_ "pnconnector/src/setting"
	//_ "dagent/src/routers"
	_ "net/http"
	_ "time"
	//	"fmt"
	"pnconnector/src/errors"
	"github.com/spf13/cobra"
)

var daemonCmd = &cobra.Command{
	Use:   "daemon",
	Short: "daemon mode",
	Long: `run on daemon mode.

Usage : 

run cli on daemon mode.



You can set ip and port number for connent to Poseidon OS using config.yaml or flags.
Default value is as below.

IP   : 127.0.0.1
Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) > 0 {
			return errors.New("no args !!!")
		}

		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		Daemon(cmd, args)
	},
}

func init() {

	if Mode == "debug" {
		rootCmd.AddCommand(daemonCmd)
	}
}

func Daemon(cmd *cobra.Command, args []string) {

	InitConnect()
	startServer()
}

func startServer() {
	//routersInit := routers.InitRouter()
	/*
		server := &http.Server{
			Addr:           ":" + setting.Config.Server.Dagent.Port,
			Handler:        routersInit,
			ReadTimeout:    30 * time.Second,
			WriteTimeout:   30 * time.Second,
			MaxHeaderBytes: 1 << 20,
		}
	*/
	//server.ListenAndServe()
}
