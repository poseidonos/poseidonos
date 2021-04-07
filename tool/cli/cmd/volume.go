package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"github.com/spf13/cobra"
)

var VolumeCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"create":       iBoFOS.CreateVolume,
	"mount":        iBoFOS.MountVolume,
	"unmount":      iBoFOS.UnmountVolume,
	"delete":       iBoFOS.DeleteVolume,
	"list":         iBoFOS.ListVolume,
	"update_qos":   iBoFOS.UpdateVolumeQoS,
	"rename":       iBoFOS.RenameVolume,
	"get_max_cnt":  iBoFOS.GetMaxVolumeCount,
	"get_host_nqn": iBoFOS.GetHostNQN,
}

var volumeCmd = &cobra.Command{
	Use:   "volume [msg]",
	Short: "Request for volume msg to Poseidon OS",
	Long: `Request for volume msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]        : [description]                                                    : [example of flag]

volume     : create       : Create a new volume in unit of bytes.                            : --name [vol name] --size [vol size] --maxiops [max iops] --maxbw [max bw] (maxiops, maxbw are optional and default value is 0.)
           : mount        : Mount a volume.                                                  : --name [vol name]
           : unmount      : Unmount a volume.                                                : --name [vol name]
           : delete       : Delete a volume.                                                 : --name [vol name]
           : list         : Listing all volumes.                                             : not needed
           : update_qos   : Update volumes QoS properties.                                   : --name [vol name] --maxiops [max iops] --maxbw [max bw] 
           : rename       : Update volume name.                                              : --name [vol name] --newname [new vol name]
           : get_max_cnt  : Get max volume count.                                            : not needed
           : get_host_nqn : Get host nqn.                                                    : not needed

			 
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

		_, exist := VolumeCommand[args[0]]

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

	rootCmd.AddCommand(volumeCmd)

	volumeCmd.PersistentFlags().StringVarP(&name, "name", "n", "", "set name \"-n vol01\"")
	volumeCmd.PersistentFlags().StringVar(&newName, "newname", "", "set new name \"--newname vol01\"")
	volumeCmd.PersistentFlags().StringVar(&size, "size", "", "set size \"--size 4194304\" or \"--size 10MB\"")
	volumeCmd.PersistentFlags().Uint64Var(&maxiops, "maxiops", 0, "set maxiops \"--maxiops 4194304\"")
	volumeCmd.PersistentFlags().Uint64Var(&maxbw, "maxbw", 0, "set maxbw \"--maxbw 4194304\"")
	volumeCmd.PersistentFlags().StringVarP(&array, "array", "a", "", "set array name")
	volumeCmd.PersistentFlags().StringVar(&subNQN, "subnqn", "", "set sub system NVMe qualified name")
}
