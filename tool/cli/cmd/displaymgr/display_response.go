package displaymgr

import (
	pb "cli/api"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"encoding/json"
	"fmt"
	"os"
	"pnconnector/src/util"
	"strconv"
	"text/tabwriter"

	"code.cloudfoundry.org/bytefmt"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/reflect/protoreflect"
)

func toByte(displayUnit bool, size uint64) string {
	if displayUnit {
		return bytefmt.ByteSize(size)
	}

	return strconv.FormatUint(size, 10)
}

func PrintProtoResponse(command string, res protoreflect.ProtoMessage) error {
	m := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}
	resByte, err := m.Marshal(res)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf response: %v", err)
		return err
	}

	resJson := string(resByte)
	PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)

	return nil
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
	if resJSON != "" {
		fmt.Println("{\"Response\":", resJSON, "}")
	}
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
				globals.FieldSeparator+"WriteThrough\t"+
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
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"----------")

		// Data
		for _, array := range res.RESULT.DATA.ARRAYLIST {
			total, _ := strconv.ParseUint(array.CAPACITY, 10, 64)
			used, _ := strconv.ParseUint(array.USED, 10, 64)
			fmt.Fprint(w,
				strconv.Itoa(array.ARRAYINDEX)+"\t"+
					globals.FieldSeparator+array.ARRAYNAME+"\t"+
					globals.FieldSeparator+array.STATUS+"\t"+
					globals.FieldSeparator+array.CREATEDATETIME+"\t"+
					globals.FieldSeparator+array.UPDATEDATETIME+"\t"+
					globals.FieldSeparator+toByte(displayUnit, total)+"\t"+
					globals.FieldSeparator+toByte(displayUnit, used)+"\t"+
					globals.FieldSeparator+strconv.FormatBool(array.WRITETHROUGH)+"\t"+
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
		total, _ := strconv.ParseUint(array.CAPACITY, 10, 64)
		used, _ := strconv.ParseUint(array.USED, 10, 64)
		fmt.Fprintln(w, "Total\t: "+toByte(displayUnit, total))
		fmt.Fprintln(w, "Used\t: "+toByte(displayUnit, used))
		fmt.Fprintln(w, "GCMode\t: "+array.GCMODE)
		fmt.Fprintln(w, "MetaRAID\t: "+array.METARAID)
		fmt.Fprintln(w, "DataRAID\t: "+array.DATARAID)
		fmt.Fprintln(w, "WriteThrough\t: "+strconv.FormatBool(array.WRITETHROUGH))
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
				globals.FieldSeparator+"Index\t"+
				globals.FieldSeparator+"UUID\t"+
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
				globals.FieldSeparator+"-------------------------------\t"+
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
					globals.FieldSeparator+strconv.Itoa(volume.INDEX)+"\t"+
					globals.FieldSeparator+volume.UUID+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.TOTAL)+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.REMAIN)+"\t"+
					globals.FieldSeparator+strconv.FormatUint(100-(volume.REMAIN*100/volume.TOTAL), 10)+"\t"+
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

		fmt.Fprintln(w, "UUID\t: "+volume.UUID)
		fmt.Fprintln(w, "Name\t: "+volume.VOLUMENAME)
		fmt.Fprintln(w, "TotalCapacity\t: "+toByte(displayUnit, volume.TOTAL))
		fmt.Fprintln(w, "RemainingCapacity\t: "+toByte(displayUnit, volume.REMAIN))
		fmt.Fprintln(w, "Used%\t: "+strconv.FormatUint(100-(volume.REMAIN*100/volume.TOTAL), 10))
		fmt.Fprintln(w, "Status\t: "+volume.STATUS)
		fmt.Fprintln(w, "MaximumIOPS\t: "+strconv.Itoa(volume.MAXIOPS))
		fmt.Fprintln(w, "MaximumBandwidth\t: "+strconv.Itoa(volume.MAXBW))
		fmt.Fprintln(w, "MinimumIOPS\t: "+strconv.Itoa(volume.MINIOPS))
		fmt.Fprintln(w, "MinimumBandwidth\t: "+strconv.Itoa(volume.MINBW))
		fmt.Fprintln(w, "SubNQN\t: "+volume.SUBNQN)
		fmt.Fprintln(w, "Array\t: "+volume.ARRAYNAME)

		fmt.Fprintln(w, "")

		w.Flush()

	case "LISTDEVICE":
		res := &pb.ListDeviceResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"SerialNumber\t"+
				globals.FieldSeparator+"Address\t"+
				globals.FieldSeparator+"Class\t"+
				globals.FieldSeparator+"ModelNumber\t"+
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
		for _, device := range res.GetResult().GetData().GetDevicelist() {
			fmt.Fprintln(w,
				device.Name+"\t"+
					globals.FieldSeparator+device.SerialNumber+"\t"+
					globals.FieldSeparator+device.Address+"\t"+
					globals.FieldSeparator+device.Class+"\t"+
					globals.FieldSeparator+device.ModelNumber+"\t"+
					globals.FieldSeparator+device.Numa+"\t"+
					globals.FieldSeparator+toByte(displayUnit, device.Size))
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
		res := &pb.GetLogLevelResponse{}
		json.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		fmt.Println("Log level: " + res.GetResult().GetData().Level)

	case "LOGGERINFO":
		res := &pb.LoggerInfoResponse{}
		json.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		data := res.GetResult().GetData()
		fmt.Fprintln(w, "minorLogPath\t: "+data.MinorLogPath)
		fmt.Fprintln(w, "majorLogPath\t: "+data.MajorLogPath)
		fmt.Fprintln(w, "logfileSizeInMb\t: "+data.LogfileSizeInMb)
		fmt.Fprintln(w, "logfileRotationCount\t:", data.LogfileRotationCount)
		fmt.Fprintln(w, "minAllowableLogLevel\t: "+data.MinAllowableLogLevel)
		fmt.Fprintln(w, "filterEnabled\t:", data.FilterEnabled == 1)
		fmt.Fprintln(w, "filterIncluded\t: "+data.FilterIncluded)
		fmt.Fprintln(w, "filterExcluded\t: "+data.FilterExcluded)
		fmt.Fprintln(w, "structuredLogging\t: "+strconv.FormatBool(data.StructuredLogging))

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
		res := &pb.ListSubsystemResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
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
		for _, subsystem := range res.GetResult().GetData().GetSubsystemlist() {
			fmt.Fprintln(w,
				subsystem.GetNqn()+"\t"+
					globals.FieldSeparator+subsystem.GetSubtype()+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.GetListenAddresses()))+"\t"+
					globals.FieldSeparator+subsystem.GetSerialNumber()+"\t"+
					globals.FieldSeparator+subsystem.GetModelNumber()+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.GetNamespaces())))
		}
		w.Flush()

	case "SUBSYSTEMINFO":
		res := &pb.SubsystemInfoResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		if len(res.GetResult().GetData().GetSubsystemlist()) != 0 {
			subsystem := res.GetResult().GetData().GetSubsystemlist()[0]

			w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

			fmt.Fprintln(w, "nqn\t: "+subsystem.GetNqn())
			fmt.Fprintln(w, "subtype\t: "+subsystem.GetSubtype())
			fmt.Fprint(w, "listen_addresses\t: ")
			for _, address := range subsystem.GetListenAddresses() {
				fmt.Fprintln(w, "")
				fmt.Fprintln(w, "\t  {")
				fmt.Fprintln(w, "\t    trtype : "+address.GetTransportType())
				fmt.Fprintln(w, "\t    adrfam : "+address.GetAddressFamily())
				fmt.Fprintln(w, "\t    traddr : "+address.GetTargetAddress())
				fmt.Fprintln(w, "\t    trsvcid : "+address.GetTransportServiceId())
				fmt.Fprint(w, "\t  }")
			}
			fmt.Fprintln(w, "")
			fmt.Fprintln(w, "allow_any_host\t:", subsystem.GetAllowAnyHost() != 0)
			fmt.Fprintln(w, "hosts\t: ")
			for _, host := range subsystem.GetHosts() {
				fmt.Fprintln(w, "\t  { nqn : "+host.GetNqn()+" }")
			}
			if "NVMe" == subsystem.GetSubtype() {
				fmt.Fprintln(w, "serial_number\t: "+subsystem.GetSerialNumber())
				fmt.Fprintln(w, "model_number\t: "+subsystem.GetModelNumber())
				fmt.Fprintln(w, "max_namespaces\t:", subsystem.GetMaxNamespaces())
				fmt.Fprint(w, "namespaces\t: ")
				for _, namespace := range subsystem.GetNamespaces() {
					fmt.Fprintln(w, "")
					fmt.Fprintln(w, "\t  {")
					fmt.Fprintln(w, "\t    nsid :", namespace.GetNsid())
					fmt.Fprintln(w, "\t    bdev_name : "+namespace.GetBdevName())
					fmt.Fprintln(w, "\t    uuid : "+namespace.GetUuid())
					fmt.Fprint(w, "\t  }")
				}
				fmt.Fprintln(w, "")
			}
			w.Flush()
		}

	case "LISTNODE":
		res := &pb.ListNodeResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"IP\t"+
				globals.FieldSeparator+"Lastseen\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t")

		// Data
		for _, node := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				node.Name+"\t"+
					globals.FieldSeparator+node.Ip+"\t"+
					globals.FieldSeparator+node.Lastseen+"\t")
		}
		w.Flush()

	case "LISTHAVOLUME":
		res := &pb.ListHaVolumeResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Id\t"+
				globals.FieldSeparator+"Name\t"+
				globals.FieldSeparator+"NodeName\t"+
				globals.FieldSeparator+"ArrayName\t"+
				globals.FieldSeparator+"Size\t"+
				globals.FieldSeparator+"Lastseen\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t")

		// Data
		for _, volume := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				strconv.FormatInt(int64(volume.Id), 10)+"\t"+
					globals.FieldSeparator+volume.Name+"\t"+
					globals.FieldSeparator+volume.NodeName+"\t"+
					globals.FieldSeparator+volume.ArrayName+"\t"+
					globals.FieldSeparator+strconv.FormatInt(volume.Size, 10)+"\t"+
					globals.FieldSeparator+volume.Lastseen+"\t")
		}
		w.Flush()

	case "LISTHAREPLICATION":
		res := &pb.ListHaReplicationResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Id\t"+
				globals.FieldSeparator+"SourceVolumeId\t"+
				globals.FieldSeparator+"SourceWalVolumeId\t"+
				globals.FieldSeparator+"DestinationVolumeId\t"+
				globals.FieldSeparator+"DestinationWalVolumeId\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t")

		// Data
		for _, replication := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				strconv.FormatInt(int64(replication.Id), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.SourceVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.SourceWalVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.DestinationVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.DestinationWalVolumeId), 10)+"\t")
		}
		w.Flush()

	case "SYSTEMINFO":
		res := &pb.SystemInfoResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "PosVersion\t: "+res.GetResult().GetData().Version)
		fmt.Fprintln(w, "BiosVersion\t: "+res.GetResult().GetData().BiosVersion)
		fmt.Fprintln(w, "BiosVendor\t: "+res.GetResult().GetData().BiosVendor)
		fmt.Fprintln(w, "BiosReleaseDate\t: "+res.GetResult().GetData().BiosReleaseDate)

		fmt.Fprintln(w, "SystemManufacturer\t: "+res.GetResult().GetData().SystemManufacturer)
		fmt.Fprintln(w, "SystemProductName\t: "+res.GetResult().GetData().SystemProductName)
		fmt.Fprintln(w, "SystemSerialNumber\t: "+res.GetResult().GetData().SystemSerialNumber)
		fmt.Fprintln(w, "SystemUuid\t: "+res.GetResult().GetData().SystemUuid)

		fmt.Fprintln(w, "BaseboardManufacturer\t: "+res.GetResult().GetData().BaseboardManufacturer)
		fmt.Fprintln(w, "BaseboardProductName\t: "+res.GetResult().GetData().BaseboardProductName)
		fmt.Fprintln(w, "BaseboardSerialNumber\t: "+res.GetResult().GetData().BaseboardSerialNumber)
		fmt.Fprintln(w, "BaseboardVersion\t: "+res.GetResult().GetData().BaseboardVersion)

		fmt.Fprintln(w, "ProcessorManufacturer\t: "+res.GetResult().GetData().ProcessorManufacturer)
		fmt.Fprintln(w, "ProcessorVersion\t: "+res.GetResult().GetData().ProcessorVersion)
		fmt.Fprintln(w, "ProcessorFrequency\t: "+res.GetResult().GetData().ProcessorFrequency)

		w.Flush()

	case "GETSYSTEMPROPERTY":
		res := messages.POSPropertyResponse{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "RebuildPerfImpact\t: "+res.RESULT.DATA.REBUILDPOLICY)

		w.Flush()
	case "GETTELEMETRYPROPERTY":
		res := &pb.GetTelemetryPropertyResponse{}
		protojson.Unmarshal([]byte(resJSON), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Status\t: "+strconv.FormatBool(res.GetResult().GetData().Status))
		fmt.Fprintln(w, "PublicationListPath\t: "+res.GetResult().GetData().PublicationListPath)

		w.Flush()

	case "STARTPOS":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
		fmt.Println(res.RESULT.STATUS.DESCRIPTION)

	case "STOPPOS":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		fmt.Println("PoseidonOS termination has been requested. PoseidonOS will be terminated soon.")

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

func isFailed(status pb.Status) bool {
	return int(status.GetCode()) != globals.CliServerSuccessCode
}

func printEvent(status pb.Status) {
	code := status.GetCode()
	name := status.GetEventName()
	desc := status.GetDescription()
	cause := status.GetCause()
	solution := status.GetSolution()

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
	fmtStr := "%s (%d) - %s\n"

	if cause != "" {
		fmtStr += "Cause: " + cause + "\n"
	}
	if solution != "" {
		fmtStr += "Solution: " + solution + "\n"
	}

	fmt.Fprintf(w, fmtStr, name, code, desc)
	w.Flush()
}

func printEventInfo(code int, name string, desc string, cause string, solution string) {
	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
	fmtStr := "%s (%d) - %s\n"

	if cause != "" {
		fmtStr += "Cause: " + cause + "\n"
	}

	if solution != "" {
		fmtStr += "Solution: " + solution + "\n"
	}

	fmt.Fprintf(w, fmtStr, name, code, desc)

	w.Flush()
}
