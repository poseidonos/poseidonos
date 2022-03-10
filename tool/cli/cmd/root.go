package cmd

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"time"

	"github.com/spf13/cast"
	"github.com/spf13/cobra"
	"github.com/zpatrick/go-bytesize"

	"cli/cmd/arraycmds"
	"cli/cmd/develcmds"
	"cli/cmd/devicecmds"
	"cli/cmd/globals"
	"cli/cmd/loggercmds"
	"cli/cmd/qoscmds"
	"cli/cmd/subsystemcmds"
	"cli/cmd/systemcmds"
	"cli/cmd/telemetrycmds"
	"cli/cmd/volumecmds"
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/setting"
	"pnconnector/src/util"
)

var isVerbose bool
var isDebug bool
var isJson bool
var isQuiet bool
var displayVersion bool

var ip string
var port string

var buffer []string
var data []string
var spare []string
var name string
var newName string
var level string
var array string
var raidType string
var subNQN string

var fttype int
var size string
var maxiops uint64
var maxbw uint64

var vol []string
var miniops uint64
var minbw uint64

var prio uint
var weight uint

var (
	PosCliVersion string
	GitCommit     string
	BuildTime     string
	RootDir       string
)

var RootCmd = &cobra.Command{
	Use:   "poseidonos-cli",
	Short: "poseidonos-cli - A command-line interface for PoseidonOS",
	Long: `poseidonos-cli - A command-line interface for PoseidonOS

	PoseidonOS command-line interface (PoseidonOS CLI) is a management tool for PoseidonOS.
	Using PoseidonOS CLI, you can start/stop PoseidonOS and manage arrays, devices, and volumes of PoseidonOS.
	
	Type --help or -h with the command to see detailed information about each command.

Syntax: 
  poseidonos-cli [global-flags] commands subcommand [flags]
		`,
	Run: func(cmd *cobra.Command, args []string) {
		if displayVersion == true {
			fmt.Println("poseidonos-cli version " + getVersion())
			os.Exit(0)
		}

		if len(args) == 0 {
			cmd.Help()
			os.Exit(0)
		}
	},
}

func getVersion() string {
	intBldTime, _ := strconv.ParseInt(BuildTime, 10, 64)
	unixTime := time.Unix(intBldTime, 0)
	return PosCliVersion + "-" + unixTime.Format("200601021")
}

func Execute() {
	if err := RootCmd.Execute(); err != nil {
		os.Exit(1)
	}
}

func init() {
	regFlags()
	regGlobalFlags()
	log.SetOutput(RootDir)
	addCmd()
}

func regFlags() {
	RootCmd.Flags().BoolVar(&displayVersion, "version", false, "Display the version information.")
}

func regGlobalFlags() {
	// Global flags
	// TODO(mj): Add verbose and quiet modes
	RootCmd.PersistentFlags().StringVar(&globals.IPv4, "ip", "127.0.0.1", "Set IPv4 address to PoseidonOS for this command.")
	RootCmd.PersistentFlags().StringVar(&globals.Port, "port", "18716", "Set the port number to PoseidonOS for this command.")
	RootCmd.PersistentFlags().BoolVar(&globals.IsDebug, "debug", false, "Print response for debug.")
	RootCmd.PersistentFlags().BoolVar(&globals.IsJSONReq, "json-req", false, "Print request in JSON form.")
	RootCmd.PersistentFlags().BoolVar(&globals.IsJSONRes, "json-res", false, "Print response in JSON form.")
	RootCmd.PersistentFlags().BoolVar(&globals.DisplayUnit, "unit", false, "Display unit (B, KB, MB, ...) when displaying capacity.")
	RootCmd.PersistentFlags().StringVar(&globals.FieldSeparator, "fs", "|", "Field separator for the output.")
}

func addCmd() {
	// Command categories
	RootCmd.AddCommand(arraycmds.ArrayCmd)
	RootCmd.AddCommand(volumecmds.VolumeCmd)
	RootCmd.AddCommand(systemcmds.SystemCmd)
	RootCmd.AddCommand(devicecmds.DeviceCmd)
	RootCmd.AddCommand(loggercmds.LoggerCmd)
	RootCmd.AddCommand(subsystemcmds.SubsystemCmd)
	RootCmd.AddCommand(develcmds.DevelCmd)
	RootCmd.AddCommand(WbtCmd)
	RootCmd.AddCommand(qoscmds.QosCmd)
	RootCmd.AddCommand(telemetrycmds.TelemetryCmd)
	RootCmd.AddCommand(completionCmd)
}

// TODO(mj): this function remains for wbt and file commands. This needs to be revised.
func InitConnect() {

	// TODO(mj): Add verbose and quiet modes
	if isVerbose == true {
		log.SetVerboseMode()
	} else if isDebug == true {
		log.SetDebugMode()
	}

	setting.LoadConfig()

	if len(ip) != 0 {
		globals.IPv4 = ip
	}

	if len(port) != 0 {
		globals.Port = port
	}

	unixIntValue, _ := strconv.ParseInt(BuildTime, 10, 64)

	log.Info("Git commit: "+GitCommit+"  Build Time: ", time.Unix(unixIntValue, 0))
	log.Info("ip, port :", setting.Config.Server.IBoF.IP, setting.Config.Server.IBoF.Port)
}

// TODO(mj): this function remains for wbt and file commands. This needs to be revised.
func PrintReqRes(req model.Request, res model.Response) {
	if isQuiet {
		return
	}

	if isJson {
		b, _ := json.Marshal(req)
		fmt.Print("{\n \"Request\":", string(b), ", \n")
		b, _ = json.Marshal(res)
		fmt.Print(" \"Response\":", string(b), "\n}\n")
	} else {
		b, _ := json.MarshalIndent(req.Param, "", "    ")

		fmt.Println("\n\nRequest to Poseidon OS")
		fmt.Println("    xrId        : ", req.Rid)
		fmt.Println("    command     : ", req.Command)

		if string(b) != "null" {
			fmt.Println("    Param       :")
			fmt.Println(string(b))
		}

		fmt.Println("\n\nResponse from Poseidon OS")
		result, err := util.GetStatusInfo(res.Result.Status.Code)

		if err == nil {
			fmt.Println("    Code         : ", result.Code)
			fmt.Println("    Level        : ", result.Level)
			fmt.Println("    Description  : ", result.Description)
			fmt.Println("    Problem      : ", result.Problem)
			fmt.Println("    Solution     : ", result.Solution)
		} else {

			fmt.Println("    Code        : ", res.Result.Status.Code)
			fmt.Println("    Description : ", res.Result.Status.Description)
			log.Infof("%v\n", err)
		}

		b, _ = json.MarshalIndent(res.Result.Data, "", "    ")

		obj := map[string]interface{}{}

		json.Unmarshal([]byte(b), &obj)

		volumes := obj["volumes"]
		if volumes != nil {
			for _, b := range volumes.([]interface{}) {
				b.(map[string]interface{})["remain"] = ChangeDataHumanReadable(cast.ToUint64(b.(map[string]interface{})["remain"]))
				b.(map[string]interface{})["total"] = ChangeDataHumanReadable(cast.ToUint64(b.(map[string]interface{})["total"]))
			}
		}
		if obj["capacity"] != nil {
			obj["capacity"] = ChangeDataHumanReadable(cast.ToUint64(obj["capacity"]))
		}
		if obj["used"] != nil {
			obj["used"] = ChangeDataHumanReadable(cast.ToUint64(obj["used"]))
		}

		if string(b) != "null" {
			str, _ := json.MarshalIndent(obj, "", "    ")
			fmt.Println("    Data         : \n", string(str))
		}

		fmt.Print("\n\n")
	}
}

func ChangeDataHumanReadable(size uint64) string {
	b := bytesize.Bytesize(size)

	if b.Gigabytes() >= 1000 {
		return fmt.Sprintf("%s (%dB)", b.Format("tb"), size)

	} else {
		return fmt.Sprintf("%s (%dB)", b.Format("gb"), size)
	}
}
