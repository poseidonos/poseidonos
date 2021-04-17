package cmd

import (
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/setting"
	"pnconnector/src/util"
	"encoding/json"
	"fmt"
	"github.com/c2h5oh/datasize"
	"github.com/google/uuid"
	"github.com/spf13/cast"
	"github.com/spf13/cobra"
	"github.com/zpatrick/go-bytesize"
	"os"
	"strconv"
	"time"
)

var isVerbose bool
var isDebug bool
var isJson bool
var isQuiet bool

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

var devType string
var numBlocks uint
var blockSize uint

var prio uint
var weight uint

var GitCommit string
var BuildTime string

var rootCmd = &cobra.Command{
	Use:   "cli",
	Short: "IBoF cli is simple client of IBoF.",
	Long: `IBoF cli is simple client of IBoF.

You can set ip and port number of iBoF using config.yaml and use a "command" command with -i flag.
		`,
	Run: func(cmd *cobra.Command, args []string) {
		if len(args) == 0 {
			cmd.Help()
			os.Exit(0)
		}
	},
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		os.Exit(1)
	}
}

func init() {

	if Mode == "debug" {

		rootCmd.PersistentFlags().BoolVar(&isVerbose, "verbose", false, "verbose output")
		rootCmd.PersistentFlags().BoolVar(&isDebug, "debug", false, "set a debug mode")
		rootCmd.PersistentFlags().BoolVar(&isJson, "json", false, "print request and response formatted json")
		rootCmd.PersistentFlags().BoolVar(&isQuiet, "quiet", false, "set a quiet mode")
	}

	rootCmd.PersistentFlags().StringVar(&ip, "ip", "", "set ip address like \"--ip 127.0.0.1\"")
	rootCmd.PersistentFlags().StringVar(&port, "port", "", "set port number like \"--port 18716\"")
}

func InitConnect() {

	if isVerbose == true {
		log.SetVerboseMode()
	} else if isDebug == true {
		log.SetDebugMode()
	}

	setting.LoadConfig()

	if len(ip) != 0 {
		setting.Config.Server.IBoF.IP = ip
	}

	if len(port) != 0 {
		setting.Config.Server.IBoF.Port = port
	}

	unixIntValue, _ := strconv.ParseInt(BuildTime, 10, 64)

	log.Info("Git commit: "+GitCommit+"  Build Time: ", time.Unix(unixIntValue, 0))

	log.Info("ip, port :", setting.Config.Server.IBoF.IP, setting.Config.Server.IBoF.Port)
}

func Send(cmd *cobra.Command, args []string) (model.Response, error) {

	var req model.Request
	var res model.Response
	var err error
	var xrId string
	newUUID, err := uuid.NewUUID()
	command := args[0]

	if err == nil {
		xrId = newUUID.String()
	}

	InitConnect()

	_, arrayExists := ArrayCommand[command]
	_, deviceExists := DeviceCommand[command]
	_, systemExists := SystemCommand[command]
	_, volumeExists := VolumeCommand[command]
	_, internalExists := InternalCommand[command]
	_, loggerExists := LoggerCommand[command]
	_, rebuildExists := RebuildCommand[command]

	if cmd.Name() == "array" && arrayExists {

		param := model.ArrayParam{}
		param.FtType = fttype

		if cmd.PersistentFlags().Changed("name") && len(name) > 0 {
			param.Name = name
		}

		if cmd.PersistentFlags().Changed("raidtype") && len(raidType) > 0 {
			param.RaidType = raidType
		}

		for _, v := range buffer {
			device := model.Device{}
			device.DeviceName = v
			param.Buffer = append(param.Buffer, device)
		}
		for _, v := range data {
			device := model.Device{}
			device.DeviceName = v
			param.Data = append(param.Data, device)
		}

		for _, v := range spare {
			device := model.Device{}
			device.DeviceName = v
			param.Spare = append(param.Spare, device)
		}

		if cmd.PersistentFlags().Changed("array") && len(array) > 0 {
			param.Array = array
		}

		req, res, err = ArrayCommand[command](xrId, param)

	} else if cmd.Name() == "device" && deviceExists {

		param := model.DeviceParam{}

		if cmd.PersistentFlags().Changed("name") && len(name) > 0 {
			param.Name = name
		}
		if cmd.PersistentFlags().Changed("type") && len(devType) > 0 {
			param.DevType = devType
		}
		if cmd.PersistentFlags().Changed("num_blocks") && numBlocks > 0 {
			param.NumBlocks = numBlocks
		}
		if cmd.PersistentFlags().Changed("block_size") && blockSize > 0 {
			param.BlockSize = blockSize
		}

		if param != (model.DeviceParam{}) {
			req, res, err = DeviceCommand[command](xrId, param)
		} else {
			req, res, err = DeviceCommand[command](xrId, nil)
		}
	} else if cmd.Name() == "volume" && volumeExists {

		param := model.VolumeParam{}

		if cmd.PersistentFlags().Changed("size") && len(size) > 0 {
			var v datasize.ByteSize
			err = v.UnmarshalText([]byte(size))

			if err != nil {
				fmt.Println("invalid data metric ", err)
				return res, err
			}

			param.Size = uint64(v)
		}

		if cmd.PersistentFlags().Changed("name") && len(name) > 0 {
			param.Name = name
		}

		if cmd.PersistentFlags().Changed("array") && len(array) > 0 {
			param.Array = array
		}

		if cmd.PersistentFlags().Changed("subnqn") && len(subNQN) > 0 {
			param.SubNQN = subNQN
		}

		if cmd.PersistentFlags().Changed("maxiops") && maxiops > 0 {
			param.Maxiops = maxiops
		}

		if cmd.PersistentFlags().Changed("maxbw") && maxbw > 0 {
			param.Maxbw = maxbw
		}

		if cmd.PersistentFlags().Changed("newname") && len(newName) > 0 {
			param.NewName = newName
		}

		if param != (model.VolumeParam{}) {
			req, res, err = VolumeCommand[command](xrId, param)
		} else {
			req, res, err = VolumeCommand[command](xrId, nil)
		}
	} else if cmd.Name() == "logger" && loggerExists {

		param := model.LoggerParam{}

		if cmd.PersistentFlags().Changed("level") && len(level) > 0 {
			param.Level = level
		}

		if param != (model.LoggerParam{}) {
			req, res, err = LoggerCommand[command](xrId, param)
		} else {
			req, res, err = LoggerCommand[command](xrId, nil)
		}
	} else if cmd.Name() == "system" && systemExists {
		req, res, err = SystemCommand[command](xrId, nil)
	} else if cmd.Name() == "internal" && internalExists {

		param := model.InternalParam{}

		if cmd.PersistentFlags().Changed("name") && len(name) > 0 {
			param.Name = name
		}

		if cmd.PersistentFlags().Changed("prio") && prio >= 0 {
			param.Prio = prio
		}

		if cmd.PersistentFlags().Changed("weight") && weight >= 0 {
			param.Weight = weight
		}

		if param != (model.InternalParam{}) {
			req, res, err = InternalCommand[command](xrId, param)
		} else {
			req, res, err = InternalCommand[command](xrId, nil)
		}
	} else if cmd.Name() == "rebuild" && rebuildExists {

		param := model.RebuildParam{}

		if cmd.PersistentFlags().Changed("level") && len(level) > 0 {
			param.Level = level
		}

		if param != (model.RebuildParam{}) {
			req, res, err = RebuildCommand[command](xrId, param)
		} else {
			req, res, err = RebuildCommand[command](xrId, nil)
		}
	}

	if err != nil {
		fmt.Println(err)
	} else {
		PrintReqRes(req, res)
	}

	return res, err
}

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
				if b.(map[string]interface{})["remain"] != nil {
					b.(map[string]interface{})["remain"] = ChangeDataHumanReadable(cast.ToUint64(b.(map[string]interface{})["remain"]))
				}
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

func ChangeDataHumanReadable(size uint64) string{
	b := bytesize.Bytesize(size)

	if b.Gigabytes() >= 1000 {
		return fmt.Sprintf("%s (%dB)", b.Format("tb"), size)
	
	} else {
		return fmt.Sprintf("%s (%dB)", b.Format("gb"), size)
	}
}
