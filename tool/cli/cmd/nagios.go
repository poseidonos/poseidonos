package cmd

import (
	_ "pnconnector/src/routers/m9k/model"
	"encoding/json"
	"fmt"
	"github.com/spf13/cobra"
	"github.com/tidwall/gjson"
	"strconv"
)

var nagiosCmd = &cobra.Command{
	Use:   "nagios",
	Short: "nagios",
	Long:  `nagios`,
	Args: func(cmd *cobra.Command, args []string) error {
		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		PrintVol(cmd, args)
	},
}

func init() {

	rootCmd.AddCommand(nagiosCmd)
	nagiosCmd.PersistentFlags().StringVar(&name, "name", "", "")
}

func PrintVol(cmd *cobra.Command, args []string) {

	isQuiet = true

	if len(args) == 0 {
		return
	}

	command := make([]string, 1)
	command[0] = args[0]

	if command[0] == "list_vol" {

		res, _ := Send(cmd, command)

		if res.Result.Data != nil {
			b, _ := json.Marshal(res.Result.Data)

			if string(b) == "null" {
				fmt.Println("CRITICAL: Fail to get the size of volumes")
			} else {
				result := "OK: Success|"

				for i := 0; i < int(gjson.Get(string(b), "volumes.#").Int()); i++ {

					value := gjson.Get(string(b), "volumes."+strconv.Itoa(i)).String()
					result += "'" + gjson.Get(value, "name").String() + "'=" + gjson.Get(value, "total").String() + "B; "
				}
				fmt.Println(result[:len(result)-2])
			}
		}
	} else if command[0] == "smart" {

		res, _ := Send(cmd, command)

		if res.Result.Data != nil {
			b, _ := json.Marshal(res.Result.Data)

			if string(b) == "null" {
				fmt.Println("CRITICAL: Fail to get the SMART info of " + name)
			} else {
				var result string

				if args[1] == "temperature" {
					result = "OK: Success temperature of " + name + "|'temperature'=" + gjson.Get(string(b), "current_temperature").String()
				} else if args[1] == "power_on_hours" {
					result = "OK: Success power_on_hours of " + name + "|'power_on_hours'=" + gjson.Get(string(b), "power_on_hours").String()
				} else if args[1] == "unsafe_shutdowns" {
					result = "OK: Success unsafe_shutdowns of " + name + "|'unsafe_shutdowns'=" + gjson.Get(string(b), "unsafe_shutdowns").String()
				}
				fmt.Println(result)
			}
		}
	}
}
