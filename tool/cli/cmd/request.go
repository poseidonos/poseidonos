package cmd

import (
	"pnconnector/src/errors"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"fmt"
	"github.com/c2h5oh/datasize"
	"github.com/google/uuid"
	"github.com/spf13/cobra"
)

var RequestArrayCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"create_array":      iBoFOS.CreateArray,
	"delete_array":      iBoFOS.DeleteArray,
	"list_array_device": iBoFOS.ListArrayDevice,
	"array_info":        iBoFOS.ArrayInfo,
	"add_dev":           iBoFOS.AddDevice,
	"remove_dev":        iBoFOS.RemoveDevice,
	"list_array":        iBoFOS.ListArray,
	"reset_mbr":         iBoFOS.ResetMbr,
}

var RequestDeviceCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"scan_dev":         iBoFOS.ScanDevice,
	"list_dev":         iBoFOS.ListDevice,
	"start_monitoring": iBoFOS.StartDeviceMonitoring,
	"stop_monitoring":  iBoFOS.StopDeviceMonitoring,
	"monitoring_state": iBoFOS.DeviceMonitoringState,
	"detach_dev":       iBoFOS.DetachDevice,
	"smart":            iBoFOS.GetSMART,
}

var RequestSystemCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"exit_ibofos":      iBoFOS.ExitiBoFOS,
	"run_ibofos":       iBoFOS.RuniBoFOS,
	"info":             iBoFOS.IBoFOSInfo,
	"mount_ibofos":     iBoFOS.MountiBoFOS,
	"unmount_ibofos":   iBoFOS.UnmountiBoFOS,
	"stop_rebuilding":  iBoFOS.StopRebuilding,
	"set_log_level":    iBoFOS.SetLogLevel,
	"get_log_level":    iBoFOS.GetLogLevel,
	"apply_log_filter": iBoFOS.ApplyLogFilter,
	"logger_info":      iBoFOS.LoggerInfo,
	//"wbt":              iBoFOS.WBT,
	//"list_wbt":         iBoFOS.ListWBT,
	//"do_gc":            iBoFOS.DoGC,
}

var RequestVolumeCommand = map[string]func(string, interface{}) (model.Request, model.Response, error){
	"create_vol":      iBoFOS.CreateVolume,
	"mount_vol":       iBoFOS.MountVolume,
	"unmount_vol":     iBoFOS.UnmountVolume,
	"delete_vol":      iBoFOS.DeleteVolume,
	"list_vol":        iBoFOS.ListVolume,
	"update_vol_qos":  iBoFOS.UpdateVolumeQoS,
	"rename_vol":      iBoFOS.RenameVolume,
	"get_max_vol_cnt": iBoFOS.GetMaxVolumeCount,
	"get_host_nqn":    iBoFOS.GetHostNQN,
	//	"update_vol":  iBoFOS.UpdateVolume,
	//"resize_vol":     iBoFOS.ResizeVolume,
}

var requestCmd = &cobra.Command{
	Use:   "request [msg]",
	Short: "** this will be deprecated **",
	Long: `Request for msg to Poseidon OS and get a response formatted by JSON.

Available msg list :

[Category] : [msg]            : [description]                                                    : [example of flag]

array      : create_array     : Provides device configuration information for configuring array. : -b [buffer devs] -d [data devs] -s [spare devs] 
           : delete_array     : Delete array.                                                    : not needed
           : list_array_device: Show all devices in the Array.                                   : not needed
           : array_info       : Show Information about Array.                                    : not needed
           : add_dev          : Add spare device to the Array.                                   : -s [spare devs]
           : remove_dev       : Remove spare device from the Array.                              : -n [dev name]

device     : scan_dev         : Scan devices in the system.                                      : not needed
           : list_dev         : Show all devices in the system.                                  : not needed
           : smart            : Get SMART from NVMe device.                                      : -n [dev name]
           : create           : Create buffer device.                                            : -t [type, pmem/uram] -n [dev name] -b [number of blocks] -s [block size]

system     : run_ibofos       : Run iBoFOS.                                                      : not needed
           : exit_ibofos      : Exit iBoFOS.                                                     : not needed
           : info             : Show current state of IbofOS.                                    : not needed
           : mount_ibofos     : Mount iBoFOS.                                                    : not needed
           : unmount_ibofos   : Unmount iBoFOS.                                                  : not needed
           : set_log_level"   : Set filtering level to logger.                                   : --level [log level]
           : get_log_level"   : Get filtering level to logger.                                   : not needed
           : apply_log_filter : Apply filtering policy to logger.                                : not needed
           : logger_info      : Get current logger settings.                                     : not needed

volume     : create_vol       : Create a new volume in unit of bytes.                            : --name [vol name] --size [vol size] --maxiops [max iops] --maxbw [max bw] (maxiops, maxbw are optional and default value is 0.)
           : mount_vol        : Mount a volume.                                                  : --name [vol name]
           : unmount_vol      : Unmount a volume.                                                : --name [vol name]
           : delete_vol       : Delete a volume.                                                 : --name [vol name]
           : list_vol         : Listing all volumes.                                             : not needed
           : update_vol_qos   : Update volumes QoS properties.                                   : --name [vol name] --maxiops [max iops] --maxbw [max bw] 
           : rename_vol       : Update volume name.                                              : --name [vol name] --newname [new vol name]
           : get_max_vol_cnt  : Get max volume count.                                            : not needed
           : get_host_nqn     : Get host nqn.                                                    : not needed

internal   : detach_dev       : Detach device from the system.                                   : -n [dev name]
           : start_monitoring : Start monitoring daemon manually.                                : not needed
           : stop_monitoring  : Stop monitoring daemon manually.                                 : not needed
           : monitoring_state : Get monitoring state.                                            : not needed
           : stop_rebuilding  : Stop rebuilding.                                                 : not needed


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

		_, arrayExists := RequestArrayCommand[args[0]]
		_, deviceExists := RequestDeviceCommand[args[0]]
		_, systemExists := RequestSystemCommand[args[0]]
		_, volumeExists := RequestVolumeCommand[args[0]]

		if !arrayExists && !deviceExists && !systemExists && !volumeExists {
			return errors.New("not available msg !!!")
		}

		return nil
	},

	Run: func(cmd *cobra.Command, args []string) {
		RequestSend(cmd, args)
	},
}

func init() {

	rootCmd.AddCommand(requestCmd)

	requestCmd.PersistentFlags().IntVarP(&fttype, "fttype", "f", 0, "set fttype \"-f 4194304\"")
	requestCmd.PersistentFlags().StringSliceVarP(&buffer, "buffer", "b", []string{}, "set buffer name \"-b uram0\"")
	requestCmd.PersistentFlags().StringSliceVarP(&data, "data", "d", []string{}, "set data name \"-d unvme-ns-0,unvme-ns-1,unvme-ns-2\"")
	requestCmd.PersistentFlags().StringSliceVarP(&spare, "spare", "s", []string{}, "set spare name \"-s unvme-ns-3\"")
	requestCmd.PersistentFlags().StringVarP(&name, "name", "n", "", "set name \"-n vol01\"")
	requestCmd.PersistentFlags().StringVar(&newName, "newname", "", "set new name \"--newname vol01\"")
	requestCmd.PersistentFlags().StringVar(&size, "size", "", "set size \"--size 4194304\"")
	requestCmd.PersistentFlags().Uint64Var(&maxiops, "maxiops", 0, "set maxiops \"--maxiops 4194304\"")
	requestCmd.PersistentFlags().Uint64Var(&maxbw, "maxbw", 0, "set maxbw \"--maxbw 4194304\"")
	requestCmd.PersistentFlags().StringVarP(&level, "level", "l", "", "set level")
	requestCmd.PersistentFlags().StringVarP(&array, "array", "a", "", "set array name")
	requestCmd.PersistentFlags().StringVarP(&raidType, "raidtype", "r", "", "set raid type")
	requestCmd.PersistentFlags().StringVar(&subNQN, "subnqn", "", "set sub system NVMe qualified name")
}

func RequestSend(cmd *cobra.Command, args []string) (model.Response, error) {

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

	_, arrayExists := RequestArrayCommand[command]
	_, deviceExists := RequestDeviceCommand[command]
	_, systemExists := RequestSystemCommand[command]
	_, volumeExists := RequestVolumeCommand[command]

	if arrayExists {

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

		req, res, err = RequestArrayCommand[command](xrId, param)
	} else if deviceExists {

		param := model.DeviceParam{}

		if cmd.PersistentFlags().Changed("name") && len(name) > 0 {
			param.Name = name
		}
		if cmd.PersistentFlags().Changed("array") && len(array) > 0 {
			param.Array = array
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
			req, res, err = RequestDeviceCommand[command](xrId, param)
		} else {
			req, res, err = RequestDeviceCommand[command](xrId, nil)
		}
	} else if systemExists {

		if cmd.PersistentFlags().Changed("level") && len(level) > 0 {
			param := model.SystemParam{}
			param.Level = level
			req, res, err = RequestSystemCommand[command](xrId, param)
		} else {
			req, res, err = RequestSystemCommand[command](xrId, nil)
		}
	} else if volumeExists {

		param := model.VolumeParam{}

		if cmd.PersistentFlags().Changed("size") && len(size) > 0 {
			var v datasize.ByteSize
			err := v.UnmarshalText([]byte(size))
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

		if param == (model.VolumeParam{}) {
			req, res, err = RequestVolumeCommand[command](xrId, nil)
		} else {
			req, res, err = RequestVolumeCommand[command](xrId, param)
		}
	}

	if err != nil {
		fmt.Println(err)
	} else {
		PrintReqRes(req, res)
	}
	return res, err
}
