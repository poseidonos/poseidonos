package displaymgr

import (
	"cli/cmd/globals"
	"cli/cmd/messages"
	"encoding/json"
	"fmt"
	"os"
	"pnconnector/src/util"
	"strconv"
	"strings"
	"text/tabwriter"

	"code.cloudfoundry.org/bytefmt"
)

func toByte(displayUnit bool, size uint64) string {
	if displayUnit {
		return bytefmt.ByteSize(size)
	}

	return strconv.FormatUint(size, 10)
}

func PrintResponse(command string, resJSON string, isDebug bool, isJSONRes bool, displayUnit bool) {
	if isJSONRes {
		printResInJSON(resJSON)
	} else if isDebug {
		printResToDebug(resJSON)
	} else {
		printResToHumanReadable(command, resJSON, displayUnit)
	}
}

func printResToDebug(resJSON string) {
	res := messages.Response{}
	json.Unmarshal([]byte(resJSON), &res)

	printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
		res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
	statusInfo, _ := util.GetStatusInfo(res.RESULT.STATUS.CODE)

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

	fmt.Fprintln(w)
	fmt.Fprintln(w, "------------ Debug message ------------")

	fmt.Fprintln(w, "Code\t: ", statusInfo.Code)
	fmt.Fprintln(w, "Level\t: ", statusInfo.Level)
	fmt.Fprintln(w, "Description\t: ", statusInfo.Description)
	fmt.Fprintln(w, "Problem\t: ", statusInfo.Problem)
	fmt.Fprintln(w, "Solution\t: ", statusInfo.Solution)
	//fmt.Fprintln(w, "Data\t: ", res.RESULT.DATA)

	w.Flush()
}

func printResInJSON(resJSON string) {
	fmt.Println("{\"Response\":", resJSON, "}")
}

// TODO(mj): Currently, the output records may have whitespace.
// It should be assured that the output records do not have whitespace to pipeline the data to awk.
func printResToHumanReadable(command string, resJSON string, displayUnit bool) {
	switch command {
	case "LISTARRAY":
		res := messages.ListArrayResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Index\t"+
				globals.FieldSeparator+"Name\t"+
				globals.FieldSeparator+"Status\t"+
				globals.FieldSeparator+"DatetimeCreated\t"+
				globals.FieldSeparator+"DatetimeUpdated\t"+
				globals.FieldSeparator+"TotalCapacity\t"+
				globals.FieldSeparator+"UsedCapacity\t"+
				globals.FieldSeparator+"RAID")

		// Horizontal line
		fmt.Fprintln(w,
			"-----\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"----------")

		// Data
		for _, array := range res.RESULT.DATA.ARRAYLIST {
			fmt.Fprint(w,
				strconv.Itoa(array.ARRAYINDEX)+"\t"+
					globals.FieldSeparator+array.ARRAYNAME+"\t"+
					globals.FieldSeparator+array.STATUS+"\t"+
					globals.FieldSeparator+array.CREATEDATETIME+"\t"+
					globals.FieldSeparator+array.UPDATEDATETIME+"\t"+
					globals.FieldSeparator+toByte(displayUnit, array.CAPACITY)+"\t"+
					globals.FieldSeparator+toByte(displayUnit, array.USED)+"\t"+
					globals.FieldSeparator+array.DATARAID)

			fmt.Fprintln(w, "")
		}
		w.Flush()

	case "ARRAYINFO":
		res := messages.ArrayInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		array := res.RESULT.DATA

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Name\t: "+array.ARRAYNAME)
		fmt.Fprintln(w, "Index\t: "+strconv.Itoa(array.ARRAYINDEX))
		fmt.Fprintln(w, "UniqueID\t: "+strconv.Itoa(array.UNIQUEID))
		fmt.Fprintln(w, "State\t: "+array.STATE)
		fmt.Fprintln(w, "Situation\t: "+array.SITUATION)
		fmt.Fprintln(w, "CreateDatetime\t: "+array.CREATEDATETIME)
		fmt.Fprintln(w, "UpdateDatetime\t: "+array.UPDATEDATETIME)
		fmt.Fprintln(w, "RebuildingProgress\t:", array.REBUILDINGPROGRESS)
		fmt.Fprintln(w, "Total\t: "+toByte(displayUnit, array.CAPACITY))
		fmt.Fprintln(w, "Used\t: "+toByte(displayUnit, array.USED))
		fmt.Fprintln(w, "GCMode\t: "+array.GCMODE)
		fmt.Fprintln(w, "MetaRAID\t: "+array.METARAID)
		fmt.Fprintln(w, "DataRAID\t: "+array.DATARAID)
		fmt.Fprint(w, "BufferDevs\t: ")

		for _, device := range array.DEVICELIST {
			if device.DEVICETYPE == "BUFFER" {
				fmt.Fprint(w, device.DEVICENAME+"\t")
			}
		}
		fmt.Fprintln(w, "")

		fmt.Fprint(w, "DataDevs\t: ")

		for _, device := range array.DEVICELIST {
			if device.DEVICETYPE == "DATA" {
				fmt.Fprint(w, device.DEVICENAME+"\t")
			}
		}

		fmt.Fprintln(w, "")

		fmt.Fprint(w, "SpareDevs\t: ")

		for _, device := range array.DEVICELIST {
			if device.DEVICETYPE == "SPARE" {
				fmt.Fprint(w, device.DEVICENAME+"\t")
			}
		}

		fmt.Fprintln(w, "")

		w.Flush()

	case "LISTVOLUME":
		res := messages.ListVolumeResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"ID\t"+
				globals.FieldSeparator+"Total\t"+
				globals.FieldSeparator+"Remaining\t"+
				globals.FieldSeparator+"Used%\t"+
				globals.FieldSeparator+"Status\t"+
				globals.FieldSeparator+"MaxIOPS\t"+
				globals.FieldSeparator+"MaxBW\t"+
				globals.FieldSeparator+"MinIOPS\t"+
				globals.FieldSeparator+"MinBW")

		// Horizontal line
		fmt.Fprintln(w,
			"---------\t"+
				globals.FieldSeparator+"--------\t"+
				globals.FieldSeparator+"-----------------\t"+
				globals.FieldSeparator+"-----------------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------")

		// Data
		for _, volume := range res.RESULT.DATA.VOLUMELIST {
			fmt.Fprintln(w,
				volume.VOLUMENAME+"\t"+
					globals.FieldSeparator+strconv.Itoa(volume.VOLUMEID)+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.TOTAL)+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.REMAIN)+"\t"+
					globals.FieldSeparator+strconv.FormatUint(volume.REMAIN*100/volume.TOTAL, 10)+"\t"+
					globals.FieldSeparator+volume.STATUS+"\t"+
					globals.FieldSeparator+strconv.Itoa(volume.MAXIOPS)+"\t"+
					globals.FieldSeparator+strconv.Itoa(volume.MAXBW)+"\t"+
					globals.FieldSeparator+strconv.Itoa(volume.MINIOPS)+"\t"+
					globals.FieldSeparator+strconv.Itoa(volume.MINBW))
		}
		w.Flush()

	case "VOLUMEINFO":
		res := messages.VolumeInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		volume := res.RESULT.DATA

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Name\t: "+volume.VOLUMENAME)
		fmt.Fprintln(w, "TotalCapacity\t: "+toByte(displayUnit, volume.TOTAL))
		fmt.Fprintln(w, "RemainingCapacity\t: "+toByte(displayUnit, volume.REMAIN))
		fmt.Fprintln(w, "Used%\t: "+strconv.FormatUint(volume.REMAIN*100/volume.TOTAL, 10))
		fmt.Fprintln(w, "Status\t: "+volume.STATUS)
		fmt.Fprintln(w, "MaximumIOPS\t: "+strconv.Itoa(volume.MAXIOPS))
		fmt.Fprintln(w, "MaximumBandwidth\t: "+strconv.Itoa(volume.MAXBW))
		fmt.Fprintln(w, "MinimumIOPS\t: "+strconv.Itoa(volume.MINIOPS))
		fmt.Fprintln(w, "MinimumBandwidth\t: "+strconv.Itoa(volume.MINBW))
		fmt.Fprintln(w, "SubNQN\t: "+volume.SUBNQN)
		fmt.Fprintln(w, "UUID\t: "+volume.UUID)
		fmt.Fprintln(w, "Array\t: "+volume.ARRAYNAME)

		fmt.Fprintln(w, "")

		w.Flush()

	case "LISTDEVICE":
		res := messages.ListDeviceResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"SerialNumber(SN)\t"+
				globals.FieldSeparator+"Address\t"+
				globals.FieldSeparator+"Class\t"+
				globals.FieldSeparator+"MN\t"+
				globals.FieldSeparator+"NUMA\t"+
				globals.FieldSeparator+"Size")

		// Horizontal line
		fmt.Fprintln(w,
			"--------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"--------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"--------------------------\t"+
				globals.FieldSeparator+"------\t"+
				globals.FieldSeparator+"------------------")

		// Data
		for _, device := range res.RESULT.DATA.DEVICELIST {
			fmt.Fprintln(w,
				device.DEVICENAME+"\t"+
					globals.FieldSeparator+device.SERIAL+"\t"+
					globals.FieldSeparator+device.ADDRESS+"\t"+
					globals.FieldSeparator+device.CLASS+"\t"+
					globals.FieldSeparator+device.MN+"\t"+
					globals.FieldSeparator+device.NUMA+"\t"+
					globals.FieldSeparator+toByte(displayUnit, device.SIZE))
		}
		w.Flush()

	case "SMARTLOG":
		res := messages.SMARTLOGResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "AvailableSpareSpace\t:", res.RESULT.DATA.AVAILABLESPARESPACE)
		fmt.Fprintln(w, "Temperature\t:", res.RESULT.DATA.TEMPERATURE)
		fmt.Fprintln(w, "DeviceReliability\t:", res.RESULT.DATA.DEVICERELIABILITY)
		fmt.Fprintln(w, "ReadOnly\t:", res.RESULT.DATA.READONLY)
		fmt.Fprintln(w, "VolatileMemoryBackup\t:", res.RESULT.DATA.VOLATILEMEMORYBACKUP)
		fmt.Fprintln(w, "CurrentTemperature\t:", res.RESULT.DATA.CURRENTTEMPERATURE)
		fmt.Fprintln(w, "AvailableSpare\t:", res.RESULT.DATA.AVAILABLESPARE)
		fmt.Fprintln(w, "AvailableSpareThreshold\t:", res.RESULT.DATA.AVAILABLESPARETHRESHOLD)
		fmt.Fprintln(w, "LifePercentageUsed\t:", res.RESULT.DATA.LIFEPERCENTAGEUSED)
		fmt.Fprintln(w, "DataUnitsRead\t:", res.RESULT.DATA.DATAUNITSREAD)
		fmt.Fprintln(w, "DataUnitsWritten\t:", res.RESULT.DATA.DATAUNITSWRITTEN)
		fmt.Fprintln(w, "HostReadCommands\t:", res.RESULT.DATA.HOSTREADCOMMANDS)
		fmt.Fprintln(w, "HostWriteCommands\t:", res.RESULT.DATA.HOSTWRITECOMMANDS)
		fmt.Fprintln(w, "ControllerBusyTime\t:", res.RESULT.DATA.CONTROLLERBUSYTIME)
		fmt.Fprintln(w, "PowerCycles\t:", res.RESULT.DATA.POWERCYCLES)
		fmt.Fprintln(w, "PowerOnHours\t:", res.RESULT.DATA.POWERONHOURS)
		fmt.Fprintln(w, "UnsafeShutdowns\t:", res.RESULT.DATA.UNSAFESHUTDOWNS)
		fmt.Fprintln(w, "unrecoverableMediaErrors\t:", res.RESULT.DATA.UNRECOVERABLEMEDIAERROS)
		fmt.Fprintln(w, "LifetimeErrorLogEntries\t:", res.RESULT.DATA.LIFETIMEERRORLOGENTRIES)
		fmt.Fprintln(w, "WarningTemperatureTime\t:", res.RESULT.DATA.WARNINGTEMPERATURETIME)
		fmt.Fprintln(w, "CriticalTemperatureTime\t:", res.RESULT.DATA.CRITICALTEMPERATURETIME)
		fmt.Fprintln(w, "TemperatureSensor1\t:", res.RESULT.DATA.TEMPERATURESENSOR1)
		fmt.Fprintln(w, "TemperatureSensor2\t:", res.RESULT.DATA.TEMPERATURESENSOR2)
		fmt.Fprintln(w, "TemperatureSensor3\t:", res.RESULT.DATA.TEMPERATURESENSOR3)
		fmt.Fprintln(w, "TemperatureSensor4\t:", res.RESULT.DATA.TEMPERATURESENSOR4)
		fmt.Fprintln(w, "TemperatureSensor5\t:", res.RESULT.DATA.TEMPERATURESENSOR5)
		fmt.Fprintln(w, "TemperatureSensor6\t:", res.RESULT.DATA.TEMPERATURESENSOR6)
		fmt.Fprintln(w, "TemperatureSensor7\t:", res.RESULT.DATA.TEMPERATURESENSOR7)
		fmt.Fprintln(w, "TemperatureSensor8\t:", res.RESULT.DATA.TEMPERATURESENSOR8)

		w.Flush()

	case "GETLOGLEVEL":
		res := messages.GetLogLevelResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		fmt.Println("Log level: " + res.RESULT.DATA.LEVEL)

	case "LOGGERINFO":
		res := messages.LoggerInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		loggerInfo := res.RESULT.DATA

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "minor_log_path\t: "+loggerInfo.MINORLOGPATH)
		fmt.Fprintln(w, "major_log_path\t: "+loggerInfo.MAJORLOGPATH)
		fmt.Fprintln(w, "logfile_size_in_mb\t: "+loggerInfo.LOGFILESIZEINBM)
		fmt.Fprintln(w, "logfile_rotation_count\t:", loggerInfo.LOGFILEROTATIONCOUNT)
		fmt.Fprintln(w, "min_allowable_log_level\t: "+loggerInfo.MINALLOWABLELOGLEVEL)
		fmt.Fprintln(w, "filter_enabled\t:", loggerInfo.FILTERENABLED == 1)
		fmt.Fprintln(w, "filter_included\t: "+loggerInfo.FILTERINCLUDED)
		fmt.Fprintln(w, "filter_excluded\t: "+loggerInfo.FILTEREXCLUDED)
		fmt.Fprintln(w, "structured_logging\t: "+strconv.FormatBool(loggerInfo.STRUCTUREDLOGGING))

		w.Flush()

	case "CREATEQOSVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	case "RESETQOSVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	case "LISTQOSPOLICIES":
		res := messages.ListQosResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if globals.CliServerSuccessCode != res.RESULT.STATUS.CODE {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.SOLUTION, res.RESULT.STATUS.CAUSE)
		} else {
			for _, array := range res.RESULT.DATA.QOSARRAYNAME {
				fmt.Println("Array Name: " + array.ARRNAME)
				fmt.Println("")
			}
			for _, rebuild := range res.RESULT.DATA.REBUILDPOLICY {
				fmt.Println("Rebuild Impact: " + rebuild.IMPACT)
			}
			for _, volume := range res.RESULT.DATA.VOLUMEQOSLIST {

				fmt.Println("Name: " + volume.VOLUMENAME)
				fmt.Println("ID: ", volume.VOLUMEID)
				fmt.Println("Minimum Iops: ", volume.MINIOPS)
				fmt.Println("Maximum Iops: ", volume.MAXIOPS)
				fmt.Println("Minimum Bw: ", volume.MINBW)
				fmt.Println("Maximum Bw: ", volume.MAXBW)
				fmt.Println("Minimum Bw Guarantee: " + volume.MINBWGUARANTEE)
				fmt.Println("Minimum IOPS Guarantee: " + volume.MINIOPSGUARANTEE)
			}
			fmt.Println("")
		}

	case "LISTSUBSYSTEM":
		res := messages.ListSubsystemResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"Subtype\t"+
				globals.FieldSeparator+"AddressCount\t"+
				globals.FieldSeparator+"SerialNumber(SN)\t"+
				globals.FieldSeparator+"ModelNumber(MN)\t"+
				globals.FieldSeparator+"NamespaceCount")

		// Horizontal line
		fmt.Fprintln(w,
			"-------------------------------------\t"+
				globals.FieldSeparator+"-----------\t"+
				globals.FieldSeparator+"------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"--------------")

		// Data
		for _, subsystem := range res.RESULT.DATA.SUBSYSTEMLIST {
			fmt.Fprintln(w,
				subsystem.NQN+"\t"+
					globals.FieldSeparator+subsystem.SUBTYPE+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.LISTENADDRESSES))+"\t"+
					globals.FieldSeparator+subsystem.SERIAL+"\t"+
					globals.FieldSeparator+subsystem.MODEL+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.NAMESPACES)))
		}

		w.Flush()

	case "SUBSYSTEMINFO":
		res := messages.ListSubsystemResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		if len(res.RESULT.DATA.SUBSYSTEMLIST) != 0 {
			subsystem := res.RESULT.DATA.SUBSYSTEMLIST[0]

			w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

			fmt.Fprintln(w, "nqn\t: "+subsystem.NQN)
			fmt.Fprintln(w, "subtype\t: "+subsystem.SUBTYPE)
			fmt.Fprint(w, "listen_addresses\t: ")
			for _, address := range subsystem.LISTENADDRESSES {
				fmt.Fprintln(w, "")
				fmt.Fprintln(w, "\t  {")
				fmt.Fprintln(w, "\t    trtype : "+address.TRANSPORTTYPE)
				fmt.Fprintln(w, "\t    adrfam : "+address.ADDRESSFAMILY)
				fmt.Fprintln(w, "\t    traddr : "+address.TARGETADDRESS)
				fmt.Fprintln(w, "\t    trsvcid : "+address.TRANSPORTSERVICEID)
				fmt.Fprint(w, "\t  }")
			}
			fmt.Fprintln(w, "")
			fmt.Fprintln(w, "allow_any_host\t:", subsystem.ALLOWANYHOST != 0)
			fmt.Fprintln(w, "hosts\t: ")
			for _, host := range subsystem.HOSTS {
				fmt.Fprintln(w, "\t  { nqn : "+host.NQN+" }")
			}
			if "NVMe" == subsystem.SUBTYPE {
				fmt.Fprintln(w, "serial_number\t: "+subsystem.SERIAL)
				fmt.Fprintln(w, "model_number\t: "+subsystem.MODEL)
				fmt.Fprintln(w, "max_namespaces\t:", subsystem.MAXNAMESPACES)
				fmt.Fprint(w, "namespaces\t: ")
				for _, namespace := range subsystem.NAMESPACES {
					fmt.Fprintln(w, "")
					fmt.Fprintln(w, "\t  {")
					fmt.Fprintln(w, "\t    nsid :", namespace.NSID)
					fmt.Fprintln(w, "\t    bdev_name : "+namespace.BDEVNAME)
					fmt.Fprintln(w, "\t    uuid : "+namespace.UUID)
					fmt.Fprint(w, "\t  }")
				}
				fmt.Fprintln(w, "")
			}
			w.Flush()
		}

	case "GETPOSINFO":
		res := messages.POSInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		fmt.Println(res.RESULT.DATA.VERSION)

	case "STARTPOS":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
		fmt.Println(res.RESULT.STATUS.DESCRIPTION)

	default:
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	}
}

func printEventInfo(code int, name string, desc string, cause string, solution string) {
	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
	fmt.Fprint(w, name+"\t-\t")
	fmt.Fprint(w, desc)
	fmt.Fprint(w, " because "+strings.ToLower(cause))
	fmt.Fprintln(w, " (solution: "+solution+")")
	w.Flush()
}
