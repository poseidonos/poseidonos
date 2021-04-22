package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"github.com/spf13/cobra"
)

var ArrayCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"create":      iBoFOS.CreateArray,
	"delete":      iBoFOS.DeleteArray,
	"list_device": iBoFOS.ListArrayDevice,
	"info":        iBoFOS.ArrayInfo,
	"add":         iBoFOS.AddDevice,
	"remove":      iBoFOS.RemoveDevice,
	"mount":       iBoFOS.MountArray,
	"unmount":     iBoFOS.UnmountArray,
	"list":        iBoFOS.ListArray,
	"reset":       iBoFOS.ResetMbr,
}

var arrayCmd = &cobra.Command{
	Use:   "array [msg]",
	Short: "Request for array msg to Poseidon OS",
	Long: `Request for array msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]       : [description]                                                    : [example of flag]

array      : create      : Provides device configuration information for configuring array. : -b [buffer devs] -d [data devs] -s [spare devs] 
		   : delete      : Delete array.                                                    : not needed
		   : list 		 : List Array.														: not needed
           : list_device : Show all devices in the Array.                                   : not needed
           : info        : Show Information about Array.                                    : not needed
           : add         : Add spare device to the Array.                                   : -s [spare devs]
           : remove      : Remove spare device from the Array.                              : -s [spare devs]
           : mount       : Mount iBoFOS.                                                    : --name [array] 
           : unmount     : Unmount iBoFOS.                                                  : --name [array]


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

		_, exist := ArrayCommand[args[0]]

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

	rootCmd.AddCommand(arrayCmd)

	arrayCmd.PersistentFlags().IntVarP(&fttype, "fttype", "f", 0, "set fttype \"-f 4194304\"")
	arrayCmd.PersistentFlags().StringSliceVarP(&buffer, "buffer", "b", []string{}, "set buffer name \"-b uram0\"")
	arrayCmd.PersistentFlags().StringSliceVarP(&data, "data", "d", []string{}, "set data name \"-d unvme-ns-0,unvme-ns-1,unvme-ns-2\"")
	arrayCmd.PersistentFlags().StringSliceVarP(&spare, "spare", "s", []string{}, "set spare name \"-s unvme-ns-3\"")
	arrayCmd.PersistentFlags().StringVarP(&name, "name", "n", "", "set name \"-n vol01\"")
	arrayCmd.PersistentFlags().StringVarP(&raidType, "raidtype", "r", "", "set raid type")
	arrayCmd.PersistentFlags().StringVarP(&array, "array", "a", "", "set array name")
}
