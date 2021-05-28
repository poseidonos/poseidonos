package displaymgr_test

import (
	"bytes"
	"cli/cmd/displaymgr"
	"cli/cmd/testmgr"
	"log"
	"testing"
)

// This testing tests if the response is parsed and displayed well in human readable form for LISTARRAY command
func TestListArrayResHumanReadable(t *testing.T) {
	var command = "LISTARRAY"
	var resJSON = `{"command":"LISTARRAY","rid":"fromCLI",
	"result":{"status":{"code":0,"description":"DONE"},
	"data":{"arrayList": [{"createDatetime": "2021-04-16 15:52:14 +0900",
	"devicelist": [{"name": "uram0","type": "BUFFER"},
	{"name": "S4H2NE0M600736 ","type": "DATA"},
	{"name": "S4H2NE0M600745","type": "DATA"},
	{"name": "S4H2NE0M600763 ","type": "DATA"}],
	"name": "ARRAY0","status":"Mounted","updateDatetime": "2021-04-16 15:52:14 +0900"},
	{"createDatetime": "2021-04-16 15:52:14 +0900","devicelist": [{"name": "uram1","type": "BUFFER"},
	{"name": "S4H2NE0M600744","type": "DATA"},{"name": "S4H2NE0M600743 ","type": "DATA"},
	{"name": "S4H2NE0M600746 ","type": "DATA"}],"name": "ARRAY1","status":"Unmounted",
	"updateDatetime": "2021-04-16 15:52:14 +0900"}]}}}`

	expected := `Name: ARRAY0
---------------------------
Datetime Created: 2021-04-16 15:52:14 +0900
Datetime Updated: 2021-04-16 15:52:14 +0900
Status: Mounted

Devices
-------------
Name: uram0
Type: BUFFER

Name: S4H2NE0M600736 
Type: DATA

Name: S4H2NE0M600745
Type: DATA

Name: S4H2NE0M600763 
Type: DATA


Name: ARRAY1
---------------------------
Datetime Created: 2021-04-16 15:52:14 +0900
Datetime Updated: 2021-04-16 15:52:14 +0900
Status: Unmounted

Devices
-------------
Name: uram1
Type: BUFFER

Name: S4H2NE0M600744
Type: DATA

Name: S4H2NE0M600743 
Type: DATA

Name: S4H2NE0M600746 
Type: DATA`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	dist := testmgr.Levenshtein([]rune(expected), []rune(output))

	// TODO(mj): Two long texts can be different slightly.
	// dist > thresholdDist should be reivsed once we find a better way to test long texts.
	thresholdDist := 5
	if dist > thresholdDist {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestArrayInfoResHumanReadable(t *testing.T) {
	var command = "ARRAYINFO"
	var resJSON = `{"command":"ARRAYINFO","rid":"fromCLI","result":{"status":{"code":0,"description":"DONE"},"data":{"name":"TargetArrayName", "state":"BUSY","situation":"REBUILDING", "rebuilding_progress":10, "capacity":120795955200, "used":107374182400, "devicelist":[{"type":"BUFFER","name":"uram0"},{"type":"DATA","name":"unvme-ns-0"},{"type":"DATA","name":"unvme-ns-1"},{"type":"DATA","name":"unvme-ns-2"},{"type":"SPARE","name":"unvme-ns-3"}]}}}`

	expected := `Name: TargetArrayName
---------------------------
State: BUSY
Situation: REBULIDING
Rebuilding Progress: 10
Total:  120795955200
Used:  107374182400

Devices
-------------
Name: uram0
Type: BUFFER

Name: unvme-ns-0
Type: DATA

Name: unvme-ns-1
Type: DATA

Name: unvme-ns-2
Type: DATA

Name: unvme-ns-3
Type: SPARE`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	dist := testmgr.Levenshtein([]rune(expected), []rune(output))

	// TODO(mj): Two long texts can be different slightly.
	// dist > thresholdDist should be reivsed once we find a better way to test long texts.
	thresholdDist := 5
	if dist > thresholdDist {
		t.Errorf("Expected: %q Output: %s", expected, output)
	}
}

func TestListDeviceResHumanReadable(t *testing.T) {
	var command = "LISTDEVICE"
	var resJSON = `{
		"command":"LISTDEVICE",
		"rid":"42a4a8a7-65bd-11eb-9cdc-005056ad6121",
		"lastSuccessTime":1612314910,
		"result":{
		   "status":{
			  "module":"",
			  "code":0,
			  "description":"list of existing devices"
		   },
		   "data":{
			  "devicelist":[
				 {
					"addr":"0000:04:00.0",
					"class":"SYSTEM",
					"mn":"VMware Virtual NVMe Disk",
					"name":"unvme-ns-0",
					"numa":"0",
					"size":68719476736,
					"sn":"VMWare NVME_0002",
					"type":"SSD"
				 },
				 {
					"addr":"0000:0c:00.0",
					"class":"SYSTEM",
					"mn":"VMware Virtual NVMe Disk",
					"name":"unvme-ns-1",
					"numa":"0",
					"size":68719476736,
					"sn":"VMWare NVME_0003",
					"type":"SSD"
				 },
				 {
					"addr":"0000:13:00.0",
					"class":"SYSTEM",
					"mn":"VMware Virtual NVMe Disk",
					"name":"unvme-ns-2",
					"numa":"0",
					"size":68719476736,
					"sn":"VMWare NVME_0000",
					"type":"SSD"
				 },
				 {
					"addr":"0000:1b:00.0",
					"class":"SYSTEM",
					"mn":"VMware Virtual NVMe Disk",
					"name":"unvme-ns-3",
					"numa":"0",
					"size":68719476736,
					"sn":"VMWare NVME_0001",
					"type":"SSD"
				 }
			  ]
		   }
		},
		"info":{
		   "capacity":0,
		   "rebuildingProgress":"0",
		   "state":"NOT_EXIST",
		   "used":0
		}
	 }`

	expected := `Name: unvme-ns-0
---------------------------
Serial Number: VMWare NVME_0002
Address: 0000:04:00.0
Class: SYSTEM
MN: VMware Virtual NVMe Disk
NUMA: 0
Size:  68719476736
Serial Number: VMWare NVME_0002

Name: unvme-ns-1
---------------------------
Serial Number: VMWare NVME_0003
Address: 0000:0c:00.0
Class: SYSTEM
MN: VMware Virtual NVMe Disk
NUMA: 0
Size:  68719476736
Serial Number: VMWare NVME_0003

Name: unvme-ns-2
---------------------------
Serial Number: VMWare NVME_0000
Address: 0000:13:00.0
Class: SYSTEM
MN: VMware Virtual NVMe Disk
NUMA: 0
Size:  68719476736
Serial Number: VMWare NVME_0000

Name: unvme-ns-3
---------------------------
Serial Number: VMWare NVME_0001
Address: 0000:1b:00.0
Class: SYSTEM
MN: VMware Virtual NVMe Disk
NUMA: 0
Size:  68719476736
Serial Number: VMWare NVME_0001`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	dist := testmgr.Levenshtein([]rune(expected), []rune(output))

	// TODO(mj): Two long texts can be different slightly.
	// dist > thresholdDist should be reivsed once we find a better way to test long texts.
	thresholdDist := 5
	if dist > thresholdDist {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestSMARTResHumanReadable(t *testing.T) {
	var command = "SMART"
	var resJSON = `{"rid":"fromCLI","result":{"status":{"module":"","code":0,"description":"DONE"},"data":{"percentage_used":"0","temperature":"11759"}}}`

	expected := `Percentage used: 0
Tempurature: 11759`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	// TODO(mj): Two long texts can be different slightly.
	// dist > thresholdDist should be reivsed once we find a better way to test long texts.
	if expected != output {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestLoggerInfoResHumanReadable(t *testing.T) {
	var command = "LOGGERINFO"
	var resJSON = `{"command":"LOGGERINFO","rid":"fromCLI","result":{"status":{"module":"","code":0,"description":"DONE"},"data":{"minor_log_path":"/etc/ibofos/log/ibofos_log.log", "major_log_path":"/etc/ibofos/log/ibof_majorlog.log", "logfile_size_in_mb":50, "logfile_rotation_count":20, "min_allowable_log_level":"info", "deduplication_enabled":true, "deduplication_sensitivity_in_msec":20, "filter_enabled":true, "filter_included":"1000-2000", "filter_excluded":""}}}`

	expected := `minor_log_path: /etc/ibofos/log/ibofos_log.log
major_log_path: /etc/ibofos/log/ibof_majorlog.log
logfile_size_in_mb: 
logfile_rotation_count:  20
min_allowable_log_level: info
deduplication_enabled:  true
deduplication_sensitivity_in_msec:  20
filter_enabled:  true
filter_included: 1000-2000
filter_excluded:`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	// TODO(mj): Two long texts can be different slightly.
	// dist > thresholdDist should be reivsed once we find a better way to test long texts.

	dist := testmgr.Levenshtein([]rune(expected), []rune(output))

	thresholdDist := 5
	if dist > thresholdDist {
		t.Errorf("Expected: %q Output: %s", expected, output)
	}
}
