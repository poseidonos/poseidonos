package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"github.com/spf13/cobra"
)

var DeviceCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"scan":  iBoFOS.ScanDevice,
	"list":  iBoFOS.ListDevice,
	"smart": iBoFOS.GetSMART,
	"create": iBoFOS.CreateDevice,
}

var deviceCmd = &cobra.Command{
	Use:   "device [msg]",
	Short: "Request for device msg to Poseidon OS",
	Long: `Request for device msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]       : [description]                                                    : [example of flag]

device     : scan        : Scan devices in the system.                                      : not needed
           : list        : Show all devices in the system.                                  : not needed
           : smart       : Get SMART from NVMe device.                                      : -n [dev name]
           : create      : Create buffer device.                                            : -t [type, pmem/uram] -n [dev name] -b [number of blocks] -s [block size]


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

		_, exist := DeviceCommand[args[0]]

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

	rootCmd.AddCommand(deviceCmd)

	deviceCmd.PersistentFlags().StringVarP(&devType, "type", "t", "", "set buffer device type. pmem / uram")
	deviceCmd.PersistentFlags().StringVarP(&name, "name", "n", "", "set name \"-n vol01\"")
	deviceCmd.PersistentFlags().UintVarP(&numBlocks, "num_blocks", "b", 0, "set number of blocks for buffer device")
	deviceCmd.PersistentFlags().UintVarP(&blockSize, "block_size", "s", 0, "set block size for buffer device")
}
