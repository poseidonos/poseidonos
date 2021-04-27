package cmd

import (
    "pnconnector/src/errors"
    iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
    "pnconnector/src/routers/m9k/model"
    "github.com/spf13/cobra"
)

var QosCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"vol_policy":       iBoFOS.QosCreateVolumePolicy,
	"vol_reset":        iBoFOS.QosResetVolumePolicy,
	"list":             iBoFOS.QosListPolicies,
}

var qosCmd = &cobra.Command{
	Use:   "qos [msg]",
	Short: "Request for qos msg to Poseidon OS",
	Long: `Request for qos msg to Poseidon OS and get a response fommated by JSON.

Available msg list :

[Category] : [msg]            : [description]                                                    : [example of flag]

qos        : vol_policy       : Create a new qos policy for a volume                             : --vol [vol name] --minbw [min bw] --maxbw [max bw] --miniops [min iops] --maxiops [max iops] --array [arrayname] (minbw, maxbw, miniops, maxiops are optional and default value is 0. To reset use value 1)
           : vol_reset        : Reset volume's all policies                                      : --vol [vol name] --array [arrayname]
           : list             : Listing all policies active                                      : --vol [vol name] --array [arrayname]

			 
If you want to input multiple flag parameter, you have to seperate with ",". 
For example, "--vol vol1,vol2,vol3". seperation by space is not allowed.


You can set ip and port number for connect to Poseidon OS using config.yaml or flags.
Default value is as below.

IP   : 127.0.0.1
Port : 18716


	  `,
	Args: func(cmd *cobra.Command, args []string) error {

		if len(args) != 1 {
			return errors.New("need an one msg !!!")
		}

		_, exist := QosCommand[args[0]]

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

	rootCmd.AddCommand(qosCmd)

	qosCmd.PersistentFlags().StringSliceVarP(&vol, "vol", "v", []string{}, "set volume name \"-v vol01\"")
	qosCmd.PersistentFlags().Uint64Var(&minbw, "minbw", 0, "set minbw \"--minbw 4194304\"")
	qosCmd.PersistentFlags().Uint64Var(&maxbw, "maxbw", 0, "set maxbw \"--maxbw 4194304\"")
	qosCmd.PersistentFlags().Uint64Var(&miniops, "miniops", 0, "set miniops \"--miniops 4194304\"")
	qosCmd.PersistentFlags().Uint64Var(&maxiops, "maxiops", 0, "set maxiops \"--maxiops 4194304\"")
	qosCmd.PersistentFlags().StringVarP(&array, "array", "a", "", "set array name")
}
