package displaymgr

import (
	"cli/cmd/messages"
	"encoding/json"
	"fmt"
	"os"
	"pnconnector/src/util"
	"strconv"
	"text/tabwriter"
)

func PrintResponse(command string, resJSON string, isDebug bool, isJSONRes bool) {
	if isDebug {
		printResToDebug(resJSON)
	} else if isJSONRes {
		printResInJSON(resJSON)
	} else {
		printResToHumanReadable(command, resJSON)
	}
}

func printResToDebug(resJSON string) {
	res := messages.Response{}
	json.Unmarshal([]byte(resJSON), &res)

	printStatus(res.RESULT.STATUS.CODE)
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
	fmt.Println(resJSON)
}

// TODO(mj): Currently, the output records may have whitespace.
// It should be assured that the output records do not have whitespace to pipeline the data to awk.
func printResToHumanReadable(command string, resJSON string) {
	switch command {
	case "LISTARRAY":
		res := messages.ListArrayResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "Index\tName\tDatetimeCreated\tDatetimeUpdated\tStatus\tDevices(Type)")
		fmt.Fprintln(w, "-----\t----------\t---------------------\t---------------------\t----------\t-----------------------------------")

		for _, array := range res.RESULT.DATA.ARRAYLIST {
			fmt.Fprint(w, strconv.Itoa(array.ARRAYINDEX)+"\t"+array.ARRAYNAME+
				"\t"+array.CREATEDATETIME+"\t"+array.UPDATEDATETIME+"\t"+array.STATUS+"\t")

			for _, device := range array.DEVICELIST {
				fmt.Fprint(w, device.SERIAL+"(")
				fmt.Fprint(w, device.DEVICETYPE+") ")
			}
			fmt.Fprintln(w, "")
		}
		w.Flush()

	case "ARRAYINFO":
		res := messages.ArrayInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)
		array := res.RESULT.DATA

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Array\t: "+array.ARRAYNAME)
		fmt.Fprintln(w, "------------------------------------")
		fmt.Fprintln(w, "Index\t: "+strconv.Itoa(array.ARRAYINDEX))
		fmt.Fprintln(w, "State\t: "+array.STATE)
		fmt.Fprintln(w, "Situation\t: "+array.SITUATION)
		fmt.Fprintln(w, "Rebuilding Progress\t:", array.REBUILDINGPROGRESS)
		fmt.Fprintln(w, "Total(byte)\t:", array.CAPACITY)
		fmt.Fprintln(w, "Used(byte)\t:", array.USED)
		fmt.Fprintln(w, "")
		fmt.Fprintln(w, "Devices")
		fmt.Fprintln(w, "Name\tType")
		fmt.Fprintln(w, "----\t------")

		for _, device := range array.DEVICELIST {
			fmt.Fprintln(w, device.DEVICENAME+"\t"+device.DEVICETYPE)
		}

		w.Flush()

	case "LISTVOLUME":
		res := messages.ListVolumeResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Name\tID\tTotalCapacity(byte)\tRemainingCapacity(byte)\tStatus\tMaximumIOPS\tMaximumBandwith")
		fmt.Fprintln(w, "---------\t-----\t----------------------------\t----------------------------\t----------\t----------------\t----------------")

		for _, volume := range res.RESULT.DATA.VOLUMELIST {
			fmt.Fprintln(w, volume.VOLUMENAME+"\t"+strconv.Itoa(volume.VOLUMEID)+
				"\t"+strconv.Itoa(volume.TOTAL)+"\t"+strconv.Itoa(volume.REMAIN)+"\t"+
				volume.STATUS+"\t"+strconv.Itoa(volume.MAXIOPS)+"\t"+strconv.Itoa(volume.MAXBW))
		}
		w.Flush()

	case "LISTDEVICE":
		res := messages.ListDeviceResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Name\tSerialNumber(SN)\tAddress\tClass\tMN\tNUMA\tSize(byte)")
		fmt.Fprintln(w, "--------------\t-------------------\t--------------\t-------------\t--------------------------\t------\t------------------")

		for _, device := range res.RESULT.DATA.DEVICELIST {
			fmt.Fprintln(w, device.DEVICENAME+"\t"+device.SERIAL+"\t"+device.ADDRESS+"\t"+device.CLASS+
				"\t"+device.MN+"\t"+device.NUMA+"\t"+strconv.Itoa(device.SIZE))
		}
		w.Flush()

	case "SMART":
		res := messages.SMARTResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "Percentage used\t:", res.RESULT.DATA.PERCENTAGEUSED)
		fmt.Fprintln(w, "Tempurature\t:", res.RESULT.DATA.TEMPERATURE)

		w.Flush()

	case "GETLOGLEVEL":
		res := messages.GetLogLevelResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		fmt.Println("Log level: " + res.RESULT.DATA.LEVEL)

	case "LOGGERINFO":
		res := messages.LoggerInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		loggerInfo := res.RESULT.DATA

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "minor_log_path\t: "+loggerInfo.MINORLOGPATH)
		fmt.Fprintln(w, "major_log_path\t: "+loggerInfo.MAJORLOGPATH)
		fmt.Fprintln(w, "logfile_size_in_mb\t: "+loggerInfo.LOGFILESIZEINBM)
		fmt.Fprintln(w, "logfile_rotation_count\t:", loggerInfo.LOGFILEROTATIONCOUNT)
		fmt.Fprintln(w, "min_allowable_log_level\t: "+loggerInfo.MINALLOWABLELOGLEVEL)
		fmt.Fprintln(w, "deduplication_enabled\t:", loggerInfo.DEDUPLICATIONENABLED)
		fmt.Fprintln(w, "deduplication_sensitivity_in_msec\t:", loggerInfo.DEDUPLICATIONSENSITIVITYINMSEC)
		fmt.Fprintln(w, "filter_enabled\t:", loggerInfo.FILTERENABLED)
		fmt.Fprintln(w, "filter_included\t: "+loggerInfo.FILTERINCLUDED)
		fmt.Fprintln(w, "filter_excluded\t: "+loggerInfo.FILTEREXCLUDED)

		w.Flush()

	case "QOSCREATEVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
		if 0 != res.RESULT.STATUS.CODE {
			printStatus(res.RESULT.STATUS.CODE)
			fmt.Println("Description: ", res.RESULT.STATUS.DESCRIPTION)
		}

	case "QOSRESETVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
		if 0 != res.RESULT.STATUS.CODE {
			printStatus(res.RESULT.STATUS.CODE)
			fmt.Println("Description: ", res.RESULT.STATUS.DESCRIPTION)
		}

	case "QOSLISTPOLICIES":
		res := messages.ListQosResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		if 0 != res.RESULT.STATUS.CODE {
			printStatus(res.RESULT.STATUS.CODE)
			fmt.Println("Description: ", res.RESULT.STATUS.DESCRIPTION)
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
				fmt.Println("Minimim Iops: ", volume.MINIOPS)
				fmt.Println("Maximum Iops: ", volume.MAXIOPS)
				fmt.Println("Minimum Bw: ", volume.MINBW)
				fmt.Println("Maximum Bw: ", volume.MAXBW)
				fmt.Println("Minimum Bw Guarantee: " + volume.MINBWGUARANTEE)
				fmt.Println("Minimum IOPS Guarantee: " + volume.MINIOPSGUARANTEE)
			}
			fmt.Println("")
		}
	default:
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)
	}
}

func printStatus(code int) {
	if code == 0 {
		// TODO(mj): Which is better way when command has been successfully executed:
		// printing out a message or nothing?
		// fmt.Println("PoseidonOS: command has been successfully executed")
	} else {
		statusInfo, _ := util.GetStatusInfo(code)
		fmt.Println("A problem occured during process with error code: " + strconv.Itoa(code))
		fmt.Println("Problem: " + statusInfo.Description)
		fmt.Println("Possible solution: " + statusInfo.Solution)
	}
}
