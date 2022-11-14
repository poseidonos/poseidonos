package displaymgr_test

import (
	"cli/cmd/displaymgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the response is parsed and displayed well in human readable form for LISTARRAY command
func TestListArrayResHumanReadable(t *testing.T) {
	var command = "LISTARRAY"
	var resJson = `{"command":"LISTARRAY", "rid":"941604e2-5693-11ed-a87c-005056adcaa2",` +
		`"result":{"status":{"code":0, "eventName":"SUCCESS",` +
		`"description":"NONE", "cause":"NONE", "solution":"NONE"},` +
		`"data":{"arrayList":[{"index":0, "uniqueId":0, "name":"POSArray",` +
		`"status":"Unmounted", "state":"", "situation":"",` +
		`"createDatetime":"2022-10-19 08:22:32 +0000", "updateDatetime":"2022-10-19 08:22:41 +0000",` +
		`"rebuildingProgress":"", "capacity":119829587559, "used":0, "gcMode":"",` +
		`"metaRaid":"", "dataRaid":"RAID5", "writeThroughEnabled":false, "devicelist":[]},` +
		`{"index":1, "uniqueId":1, "name":"POSArray2",` +
		`"status":"Unmounted", "state":"", "situation":"",` +
		`"createDatetime":"2022-10-19 08:22:32 +0000", "updateDatetime":"2022-10-19 08:22:41 +0000",` +
		`"rebuildingProgress":"", "capacity":219829587559, "used":103012340, "gcMode":"",` +
		`"metaRaid":"", "dataRaid":"RAID5", "writeThroughEnabled":false, "devicelist":[]}` +
		`]}},` +
		`"info":{"version":"v0.12.0-rc1"}}`

	expected := `Index |Name       |Status     |DatetimeCreated           |DatetimeUpdated           |TotalCapacity |UsedCapacity  |WriteThrough  |RAID
----- |---------- |---------- |---------------------     |---------------------     |------------- |------------- |------------- |----------
0     |POSArray   |Unmounted  |2022-10-19 08:22:32 +0000 |2022-10-19 08:22:41 +0000 |119829587559  |0             |false         |RAID5
1     |POSArray2  |Unmounted  |2022-10-19 08:22:32 +0000 |2022-10-19 08:22:41 +0000 |219829587559  |103012340     |false         |RAID5
`

	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestListVolumeResHumanReadable(t *testing.T) {
	var command = "LISTVOLUME"
	var resJson = `{"command":"LISTVOLUME","rid":"fromCLI",` +
		`"result":{"status":{"code":0,"description":"DONE"},` +
		`"data":{ "array":"POSArray0",` +
		` "volumes":[{"name":"vol1","id":0,"total":214748364800,` +
		`"remain":214748364800,"status":"Mounted","maxiops":0,"maxbw":0},` +
		`{"name":"vol2","id":1,"total":11474836480,"remain":8474836480,` +
		`"status":"Unmounted","maxiops":0,"maxbw":0}]}}}`

	expected := `Name      |ID       |Total             |Remaining         |Used%     |Status     |MaxIOPS   |MaxBW     |MinIOPS   |MinBW
--------- |-------- |----------------- |----------------- |--------- |---------- |--------- |--------- |--------- |---------
vol1      |0        |214748364800      |214748364800      |0         |Mounted    |0         |0         |0         |0
vol2      |1        |11474836480       |8474836480        |27        |Unmounted  |0         |0         |0         |0
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestArrayInfoResHumanReadable(t *testing.T) {
	var command = "ARRAYINFO"
	var resJson = `{"command":"ARRAYINFO","rid":"fromCLI",` +
		`"result":{"status":{"code":0,"description":"DONE"},` +
		`"data":{"index":3,"uniqueId":13241241,"name":"TargetArrayName", ` +
		`"state":"BUSY","situation":"REBUILDING", "rebuildingProgress":"76", ` +
		`"capacity":120795955200, "used":107374182400, ` +
		`"devicelist":[{"type":"BUFFER","name":"uram0"}, ` +
		`{"type":"DATA","name":"unvme-ns-0"},{"type":"DATA","name":"unvme-ns-1"},` +
		`{"type":"DATA","name":"unvme-ns-2"},{"type":"SPARE","name":"unvme-ns-3"}]}}}`

	expected := `Name               :  TargetArrayName
Index              :  3
UniqueID           :  13241241
State              : BUSY
Situation          : REBUILDING
CreateDatetime     : 
UpdateDatetime     : 
RebuildingProgress : 76
Total              : 120795955200
Used               : 107374182400
GCMode             : 
MetaRAID           : 
DataRAID           : 
WriteThrough       : false
BufferDevs         : uram0      
DataDevs           : unvme-ns-0 unvme-ns-1 unvme-ns-2 
SpareDevs          : unvme-ns-3 
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestListDeviceResHumanReadable(t *testing.T) {
	var command = "LISTDEVICE"
	var resJson = `{
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

	expected := `Name           |SerialNumber(SN)    |Address        |Class         |MN                         |NUMA   |Size(byte)
-------------- |------------------- |-------------- |------------- |-------------------------- |------ |------------------
unvme-ns-0     |VMWare NVME_0002    |0000:04:00.0   |SYSTEM        |VMware Virtual NVMe Disk   |0      |68719476736
unvme-ns-1     |VMWare NVME_0003    |0000:0c:00.0   |SYSTEM        |VMware Virtual NVMe Disk   |0      |68719476736
unvme-ns-2     |VMWare NVME_0000    |0000:13:00.0   |SYSTEM        |VMware Virtual NVMe Disk   |0      |68719476736
unvme-ns-3     |VMWare NVME_0001    |0000:1b:00.0   |SYSTEM        |VMware Virtual NVMe Disk   |0      |68719476736
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestSMARTResHumanReadable(t *testing.T) {
	var command = "SMART"
	var resJson = `{"rid":"fromCLI","result":{"status":{"module":"",` +
		`"code":0,"description":"DONE"},"data":{"percentage_used":"28","temperature":"11759"}}}`

	expected := `Percentage used : 0
Tempurature     : 11759
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}

}

func TestLoggerInfoResHumanReadable(t *testing.T) {
	var command = "LOGGERINFO"
	var resJson = `{"command":"LOGGERINFO","rid":"fromCLI",` +
		`"result":{"status":{"module":"","code":0,"description":"DONE"},` +
		`"data":{"minor_log_path":"/etc/ibofos/log/ibofos_log.log",` +
		` "major_log_path":"/etc/ibofos/log/ibof_majorlog.log",` +
		` "logfile_size_in_mb":50, "logfile_rotation_count":20,` +
		` "min_allowable_log_level":"info", "filter_enabled":0,` +
		` "filter_included":"1000-2000", "filter_excluded":"","structured_logging":"true"}}}`

	expected := `minor_log_path          : /etc/ibofos/log/ibofos_log.log
major_log_path          : /etc/ibofos/log/ibof_majorlog.log
logfile_size_in_mb      : 
logfile_rotation_count  : 20
min_allowable_log_level : info
filter_enabled          : false
filter_included         : 1000-2000
filter_excluded         : 
structured_logging      : false
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

// Print response to stdout and hook it to a string variable
func hookResponse(command string, resJson string, isDebug bool, isJSONRes bool) string {
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	displaymgr.PrintResponse(command, resJson, false, false, false)

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	return string(out)
}

func TestVolumeInfoResHumanReadable(t *testing.T) {
	var command = "VOLUMEINFO"
	var resJson = `{"command":"VOLUMEINFO","rid":"3e44809a-7a90-11ec-b913-005056adcaa2","result":{"status":{"code":0,"description":"information of volume: vol1 of array: POSArray"},"data":{"name":"vol1","uuid":"3da91570-86b5-44f5-b6c6-b28e17883723","total":107374182400,"status":"Unmounted","maxiops":0,"maxbw":0,"minbw":0,"miniops":0,"subnqn":"","array_name":"POSArray"}},"info":{"version":"v0.10.6"}}`

	expected := `Name              : vol1
TotalCapacity     : 107374182400
RemainingCapacity : 0
Used%             : 100
Status            : Unmounted
MaximumIOPS       : 0
MaximumBandwidth  : 0
MinimumIOPS       : 0
MinimumBandwidth  : 0
SubNQN            : 
UUID              : 3da91570-86b5-44f5-b6c6-b28e17883723
Array             : POSArray

`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}

func TestWhenPosIsNotRunning(t *testing.T) {
	var command = "ARRAYLIST"
	var resJson = `{"rid":"58ddcfa1-b3f2-11ec-860b-b42e99ff989b","command":"LISTARRAY","result":{"status":{"code":-1,"eventName":"CLI_CONNECTION_ERROR","description":"dial tcp 127.0.0.1:18716: connect: connection refused","cause":"","solution":""}},"info":{"version":""}}`

	expected := `CLI_CONNECTION_ERROR - dial tcp 127.0.0.1:18716: connect: connection refused because  (solution: )
`
	output := hookResponse(command, resJson, false, false)

	if output != expected {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}
