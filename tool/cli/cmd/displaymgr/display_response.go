package displaymgr

import (
	"cli/cmd/messages"
	"encoding/json"
	"fmt"
	"log"
	"pnconnector/src/util"
	"strconv"
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

	log.Println()
	log.Println("------------ Debug message ------------")

	log.Println("Code         : ", statusInfo.Code)
	log.Println("Level        : ", statusInfo.Level)
	log.Println("Description  : ", statusInfo.Description)
	log.Println("Problem      : ", statusInfo.Problem)
	log.Println("Solution     : ", statusInfo.Solution)
	//log.Println("Data         : ", res.RESULT.DATA)
}

func printResInJSON(resJSON string) {
	log.Println(resJSON)
}

func printResToHumanReadable(command string, resJSON string) {
	switch command {
	case "LISTARRAY":
		res := messages.ListArrayResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		for _, array := range res.RESULT.DATA.ARRAYLIST {
			log.Println("Name: " + array.ARRAYNAME)
			log.Println("---------------------------")
			log.Println("Datetime Created: " + array.CREATEDATETIME)
			log.Println("Datetime Updated: " + array.UPDATEDATETIME)
			log.Println("Status: " + array.STATUS)
			log.Println("")
			log.Println("Devices")
			log.Println("-------------")
			for _, device := range array.DEVICELIST {
				log.Println("SN: " + device.SERIAL)
				log.Println("Type: " + device.DEVICETYPE)
				log.Println("")
			}
			log.Println("")
		}
	case "ARRAYINFO":
		res := messages.ArrayInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		array := res.RESULT.DATA

		log.Println("Name: " + array.ARRAYNAME)
		log.Println("---------------------------")
		log.Println("State: " + array.STATE)
		log.Println("Situation: " + array.SITUATION)
		log.Println("Rebuilding Progress: ", array.REBUILDINGPROGRESS)
		log.Println("Total: ", array.CAPACITY)
		log.Println("Used: ", array.USED)
		log.Println("")
		log.Println("Devices")
		log.Println("-------------")

		for _, device := range array.DEVICELIST {
			log.Println("Name: " + device.DEVICENAME)
			log.Println("Type: " + device.DEVICETYPE)
			log.Println("")
		}
		log.Println("")
	case "LISTVOLUME":
		res := messages.ListVolumeResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		log.Println(res)

		log.Println("Array name: " + res.RESULT.DATA.ARRAYNAME)
		log.Println("---------------------------")

		for _, volume := range res.RESULT.DATA.VOLUMELIST {
			log.Println("Name: " + volume.VOLUMENAME)
			log.Println("ID: " + volume.VOLUMEID)
			log.Println("Total capacity: ", volume.TOTAL)
			log.Println("Remaining capacity: ", volume.REMAIN)
			log.Println("Status: " + volume.STATUS)
			log.Println("Maximum IOPS: ", volume.MAXIOPS)
			log.Println("Maximum bandwidth: ", volume.MAXBW)

			log.Println("")
		}
		log.Println("")

	case "LISTDEVICE":
		res := messages.ListDeviceResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		for _, device := range res.RESULT.DATA.DEVICELIST {
			log.Println("Name: " + device.DEVICENAME)
			log.Println("---------------------------")
			log.Println("Serial Number: " + device.SERIAL)
			log.Println("Address: " + device.ADDRESS)
			log.Println("Class: " + device.CLASS)
			log.Println("MN: " + device.MN)
			log.Println("NUMA: " + device.NUMA)
			log.Println("Size: ", device.SIZE)
			log.Println("Serial Number: " + device.SERIAL)
			log.Println("")
		}
		log.Println("")

	case "SMART":
		res := messages.SMARTResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		log.Print("Percentage used: ", res.RESULT.DATA.PERCENTAGEUSED)
		log.Print("Tempurature: ", res.RESULT.DATA.TEMPERATURE)

	case "GETLOGLEVEL":
		res := messages.GetLogLevelResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		log.Print("Log level: ")
		log.Println(res.RESULT.DATA.LEVEL)

	case "LOGGERINFO":
		res := messages.LoggerInfoResponse{}
		json.Unmarshal([]byte(resJSON), &res)
		printStatus(res.RESULT.STATUS.CODE)

		loggerInfo := res.RESULT.DATA

		log.Println("minor_log_path: " + loggerInfo.MINORLOGPATH)
		log.Println("major_log_path: " + loggerInfo.MAJORLOGPATH)
		log.Println("logfile_size_in_mb: " + loggerInfo.LOGFILESIZEINBM)
		log.Println("logfile_rotation_count: ", loggerInfo.LOGFILEROTATIONCOUNT)
		log.Println("min_allowable_log_level: " + loggerInfo.MINALLOWABLELOGLEVEL)
		log.Println("deduplication_enabled: ", loggerInfo.DEDUPLICATIONENABLED)
		log.Println("deduplication_sensitivity_in_msec: ", loggerInfo.DEDUPLICATIONSENSITIVITYINMSEC)
		log.Println("filter_enabled: ", loggerInfo.FILTERENABLED)
		log.Println("filter_included: " + loggerInfo.FILTERINCLUDED)
		log.Println("filter_excluded: " + loggerInfo.FILTEREXCLUDED)

		log.Println("")

	case "QOSCREATEVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
        if (0 != res.RESULT.STATUS.CODE) {
		    printStatus(res.RESULT.STATUS.CODE)
            fmt.Println("Description: ",res.RESULT.STATUS.DESCRIPTION)
        }

	case "QOSRESETVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJSON), &res)
        if (0 != res.RESULT.STATUS.CODE) {
		    printStatus(res.RESULT.STATUS.CODE)
            fmt.Println("Description: ",res.RESULT.STATUS.DESCRIPTION)
        }

	case "QOSLISTPOLICIES":
	    res := messages.ListQosResponse{}
	    json.Unmarshal([]byte(resJSON), &res)
        if (0 != res.RESULT.STATUS.CODE) {
            printStatus(res.RESULT.STATUS.CODE)
            fmt.Println("Description: ",res.RESULT.STATUS.DESCRIPTION)
        } else {
            for _, array := range res.RESULT.DATA.QOSARRAYNAME {
                log.Println("Array Name: " + array.ARRNAME)
                log.Println("")
            }
            for _, rebuild := range res.RESULT.DATA.REBUILDPOLICY {
                log.Println("Rebuild Impact: " + rebuild.IMPACT)
            }
            for _, volume := range res.RESULT.DATA.VOLUMEQOSLIST {

                log.Println("Name: " + volume.VOLUMENAME)
                log.Println("ID: ",  volume.VOLUMEID)
                log.Println("Minimim Iops: ", volume.MINIOPS)
                log.Println("Maximum Iops: ", volume.MAXIOPS)
                log.Println("Minimum Bw: ",  volume.MINBW)
                log.Println("Maximum Bw: ", volume.MAXBW)
                log.Println("Minimum Bw Guarantee: "  + volume.MINBWGUARANTEE)
                log.Println("Minimum IOPS Guarantee: " +  volume.MINIOPSGUARANTEE)

                log.Println("")
            }
            log.Println("")
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
		fmt.Println("A problem occured during process with error code " +
			strconv.Itoa(code) + " - " + statusInfo.Description)

		fmt.Println("Possible solution: " + statusInfo.Solution)
	}
}
