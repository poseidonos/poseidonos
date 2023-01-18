/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package model

var RequesterName = "DAgent"

type Request struct {
	Command   string      `json:"command"`
	Rid       string      `json:"rid"`
	Param     interface{} `json:"param,omitempty"`
	Requester string      `json:"requester"`
}

type Response struct {
	Rid             string      `json:"rid"`
	LastSuccessTime int64       `json:"lastSuccessTime"`
	Result          Result      `json:"result"`
	Info            interface{} `json:"info,omitempty"`
}

type Result struct {
	Status Status      `json:"status"`
	Data   interface{} `json:"data,omitempty"`
}

type Status struct {
	Module         string      `json:"module"`
	Code           int         `json:"code"`
	Level          string      `json:"level,omitempty"`
	EVENTNAME      string      `json:"eventName"`
	CAUSE          string      `json:"cause"`
	Description    string      `json:"description"`
	PosDescription string      `json:"posDescription"`
	Problem        string      `json:"problem,omitempty"`
	Solution       string      `json:"solution,omitempty"`
	ErrorInfo      interface{} `json:"errorInfo,omitempty"`
}

type Device struct {
	DeviceName string `json:"deviceName"`
}

type SystemParam struct {
	Level string `json:"level,omitempty"`
}

type LoggerParam struct {
	Level string `json:"level,omitempty"`
}

type SetSystemPropertyRequest struct {
	Level string `json:"level,omitempty"`
}

type LoggerPreference struct {
	StructuredLogging string `json:"structuredLogging,omitempty"`
}

type ArrayParam struct {
	FtType   int      `json:"fttype,omitempty"`
	Name     string   `json:"name,omitempty"`
	RaidType string   `json:"raidtype,omitempty"`
	Buffer   []Device `json:"buffer,omitempty"`
	Data     []Device `json:"data,omitempty"`
	Spare    []Device `json:"spare,omitempty"`
	Array    string   `json:"array,omitempty"`
	Device   string   `json:"device,omitempty"`
}

type AutocreateArrayParam struct {
	ARRAYNAME    string            `json:"name"`
	BUFFER       [1]DeviceNameList `json:"buffer"`
	NUMDATADEVS  int               `json:"numData"`
	NUMSPAREDEVS int               `json:"numSpare,omitempty"`
	RAID         string            `json:"raidtype,omitempty"`
}
type DeviceNameList struct {
	DEVICENAME string `json:"deviceName,omitempty"`
}
type CreateDeviceReqParam struct {
	DEVICENAME string `json:"name"`
	NUMBLOCKS  int    `json:"numBlocks"`
	BLOCKSIZE  int    `json:"blockSize"`
	DEVICETYPE string `json:"devType"`
	NUMA       int    `json:"numa"`
}
type SubSystemParam struct {
	TRANSPORTTYPE      string `json:"transportType,omitempty"`
	BUFCACHESIZE       int    `json:"bufCacheSize,omitempty"`
	NUMSHAREDBUF       int    `json:"numSharedSuf,omitempty"`
	TARGETADDRESS      string `json:"targetAddress"`
	TRANSPORTSERVICEID string `json:"transportServiceId"`
	NQN                string `json:"nqn,omitempty"`
	SUBNQN             string `json:"subnqn,omitempty"`
	SERIAL             string `json:"serialNumber,omitempty"`
	MODEL              string `json:"modelNumber,omitempty"`
	MAXNAMESPACES      int    `json:"maxNamespaces,omitempty"`
	ALLOWANYHOST       bool   `json:"allowAnyHost,omitempty"`
	ANAREPORTING       bool   `json:"anaReporting,omitempty"`
}
type SetTelemetryPropertyRequest_Param struct {
	PublicationListPath string `protobuf:"bytes,1,opt,name=publicationListPath,proto3" json:"publicationListPath,omitempty"`
}

type CreateDeviceRequest_Param struct {
	Name      string `protobuf:"bytes,1,opt,name=name,proto3" json:"name,omitempty"`
	NumBlocks uint32 `protobuf:"varint,2,opt,name=numBlocks,proto3" json:"numBlocks,omitempty"`
	BlockSize uint32 `protobuf:"varint,3,opt,name=blockSize,proto3" json:"blockSize,omitempty"`
	DevType   string `protobuf:"bytes,4,opt,name=devType,proto3" json:"devType,omitempty"`
	Numa      uint32 `protobuf:"varint,5,opt,name=numa,proto3" json:"numa,omitempty"`
}
type CreateSubsystemRequest_Param struct {
	Nqn           string `protobuf:"bytes,1,opt,name=nqn,proto3" json:"nqn,omitempty"`
	SerialNumber  string `protobuf:"bytes,2,opt,name=serialNumber,proto3" json:"serialNumber,omitempty"`
	ModelNumber   string `protobuf:"bytes,3,opt,name=modelNumber,proto3" json:"modelNumber,omitempty"`
	MaxNamespaces uint32 `protobuf:"varint,4,opt,name=maxNamespaces,proto3" json:"maxNamespaces,omitempty"`
	AllowAnyHost  bool   `protobuf:"varint,5,opt,name=allowAnyHost,proto3" json:"allowAnyHost,omitempty"`
	AnaReporting  bool   `protobuf:"varint,6,opt,name=anaReporting,proto3" json:"anaReporting,omitempty"`
}
type AddListenerRequest_Param struct {
	Subnqn             string `protobuf:"bytes,1,opt,name=subnqn,proto3" json:"subnqn,omitempty"`
	TransportType      string `protobuf:"bytes,2,opt,name=transportType,proto3" json:"transportType,omitempty"`
	TargetAddress      string `protobuf:"bytes,3,opt,name=targetAddress,proto3" json:"targetAddress,omitempty"`
	TransportServiceId string `protobuf:"bytes,4,opt,name=transportServiceId,proto3" json:"transportServiceId,omitempty"`
}
type CreateTransportRequest_Param struct {
	TransportType string `protobuf:"bytes,1,opt,name=transportType,proto3" json:"transportType,omitempty"`
	BufCacheSize  int32  `protobuf:"varint,2,opt,name=bufCacheSize,proto3" json:"bufCacheSize,omitempty"`
	NumSharedBuf  int32  `protobuf:"varint,3,opt,name=numSharedBuf,proto3" json:"numSharedBuf,omitempty"`
}

type RebuildArrayRequest_Param struct {
	Name string `protobuf:"bytes,1,opt,name=name,proto3" json:"name,omitempty"`
}

type MAgentParam struct {
	ArrayIds  string `form:"arrayids"`
	VolumeIds string `form:"volumeids"`
	Time      string `form:"time"`
}

type DeviceParam struct {
	Name  string `json:"name,omitempty"`
	Spare string `json:"spare,omitempty"`
	Array string `json:"array,omitempty"`
}
type VolumeParam struct {
	Name               string `json:"name,omitempty"`
	Volume             string `json:"volume,omitempty"`
	NewName            string `json:"newname,omitempty"`
	Array              string `json:"array,omitempty"`
	SubNQN             string `json:"subnqn,omitempty"`
	Size               uint64 `json:"size,omitempty"`
	Miniops            uint64 `json:"miniops,omitempty"`
	Maxiops            uint64 `json:"maxiops,omitempty"`
	Minbw              uint64 `json:"minbw,omitempty"`
	Maxbw              uint64 `json:"maxbw,omitempty"`
	NameSuffix         uint64 `json:"namesuffix,omitempty"`
	TotalCount         uint64 `json:"totalcount,omitempty"`
	StopOnError        bool   `json:"stoponerror,omitempty"`
	MountAll           bool   `json:"mountall,omitempty"`
	TRANSPORTTYPE      string `json:"transport_type,omitempty"`
	TARGETADDRESS      string `json:"target_address"`
	TRANSPORTSERVICEID string `json:"transport_service_id"`
	ISWALVOL           bool   `json:"iswalvol"`
}

type CallbackMultiVol struct {
	TotalCount    int
	Pass          int
	Fail          int
	MultiVolArray []Response
}

type WBTParam struct {
	TestName string  `json:"testname,omitempty"`
	Argv     WBTArgv `json:"argv"`
}

type InternalParam struct {
	Name   string `json:"name,omitempty"`
	Prio   uint   `json:"prio"`
	Weight uint   `json:"weight,omitempty"`
}

type RebuildParam struct {
	Level string `json:"level,omitempty"`
}

type WBTArgv struct {
	Name      string `json:"name,omitempty"`
	Input     string `json:"input,omitempty"`
	Output    string `json:"output,omitempty"`
	Integrity string `json:"integrity,omitempty"`
	Access    string `json:"access,omitempty"`
	Operation string `json:"operation,omitempty"`
	Rba       string `json:"rba,omitempty"`
	Lba       string `json:"lba,omitempty"`
	Vsid      string `json:"vsid,omitempty"`
	Lsid      string `json:"lsid,omitempty"`
	Offset    string `json:"offset,omitempty"`
	Size      string `json:"size,omitempty"`
	Count     string `json:"count,omitempty"`
	Pattern   string `json:"pattern,omitempty"`
	Loc       string `json:"loc,omitempty"`
	Fd        string `json:"fd,omitempty"`
	Dev       string `json:"dev,omitempty"`
	Normal    string `json:"normal,omitempty"`
	Urgent    string `json:"urgent,omitempty"`
}

type BuildInfo struct {
	GitHash   string `json:"githash"`
	BuildTime string `json:"buildTime"`
}

//type SMART struct {
//	AvailableSpare           string `json:"available_spare,omitempty"`
//	AvailableSpareSpace      string `json:"available_spare_space,omitempty"`
//	AvailableSpareThreshold  string `json:"available_spare_threshold,omitempty"`
//	ContollerBusyTime        string `json:"contoller_busy_time,omitempty"`
//	CriticalTemperatureTime  string `json:"critical_temperature_time,omitempty"`
//	CurrentTemperature       string `json:"current_temperature,omitempty"`
//	DataUnitsRead            string `json:"data_units_read,omitempty"`
//	DataUnitsWritten         string `json:"data_units_written,omitempty"`
//	DeviceReliability        string `json:"device_reliability,omitempty"`
//	HostReadCommands         string `json:"host_read_commands,omitempty"`
//	HostWriteCommands        string `json:"host_write_commands,omitempty"`
//	LifPercentageUsed        string `json:"life_percentage_used,omitempty"`
//	LifetimeErrorLogEntries  string `json:"lifetime_error_log_entries,omitempty"`
//	PowerCycles              string `json:"power_cycles,omitempty"`
//	PowerOnHours             string `json:"power_on_hours,omitempty"`
//	ReadOnly                 string `json:"read_only,omitempty"`
//	Temperature              string `json:"temperature,omitempty"`
//	UnrecoverableMediaErrors string `json:"unrecoverable_media_errors,omitempty"`
//	UnsafeShutdowns          string `json:"unsafe_shutdowns,omitempty"`
//	VolatileMemoryBackup     string `json:"volatile_memory_backup,omitempty"`
//	WarningTemperatureTime   string `json:"warning_temperature_time,omitempty"`
//}
