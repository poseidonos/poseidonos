package model

type Request struct {
	Command string      `json:"command"`
	Rid     string      `json:"rid"`
	Param   interface{} `json:"param,omitempty"`
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
	Module      string `json:"module"`
	Code        int    `json:"code"`
	Level       string `json:"level,omitempty"`
	Description string `json:"description"`
	Problem     string `json:"problem,omitempty"`
	Solution    string `json:"solution,omitempty"`
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

type ArrayParam struct {
	FtType   int      `json:"fttype,omitempty"`
	Name     string   `json:"name,omitempty"`
	RaidType string   `json:"raidtype,omitempty"`
	Buffer   []Device `json:"buffer,omitempty"`
	Data     []Device `json:"data,omitempty"`
	Spare    []Device `json:"spare,omitempty"`
	Array    string   `json:"array,omitempty"`
}

type MAgentParam struct {
	Time  string
	Level string
}

type DeviceParam struct {
	Name      string `json:"name,omitempty"`
	Spare     string `json:"spare,omitempty"`
	Array     string `json:"array,omitempty"`
	DevType   string `json:"dev_type,omitempty"`
	NumBlocks uint   `json:"num_blocks,omitempty"`
	BlockSize uint   `json:"block_size,omitempty"`
}
type VolumeParam struct {
	Name        string `json:"name,omitempty"`
	NewName     string `json:"newname,omitempty"`
	Array       string `json:"array,omitempty"`
	SubNQN      string `json:"subnqn,omitempty"`
	Size        uint64 `json:"size,omitempty"`
	Maxiops     uint64 `json:"maxiops,omitempty"`
	Maxbw       uint64 `json:"maxbw,omitempty"`
	NameSuffix  uint64 `json:"namesuffix,omitempty"`
	TotalCount  uint64 `json:"totalcount,omitempty"`
	StopOnError bool   `json:"stoponerror,omitempty"`
	MountAll    bool   `json:"mountall,omitempty"`
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
	Filetype  string `json:"filetype,omitempty"`
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
	Op        string `json:"op,omitempty"`
	Cns       string `json:"cns,omitempty"`
	Nsid      string `json:"nsid,omitempty"`
	Cdw10     string `json:"cdw10,omitempty"`
	Cdw11     string `json:"cdw11,omitempty"`
	Cdw12     string `json:"cdw12,omitempty"`
	Cdw13     string `json:"cdw13,omitempty"`
	Cdw14     string `json:"cdw14,omitempty"`
	Cdw15     string `json:"cdw15,omitempty"`
	Lbaf      string `json:"lbaf,omitempty"`
	Ms        string `json:"ms,omitempty"`
	Pi        string `json:"pi,omitempty"`
	Pil       string `json:"pil,omitempty"`
	Ses       string `json:"ses,omitempty"`
	Array     string `json:"array,omitempty"`
	Volume    string `json:"volume,omitempty"`
	Module    string `json:"module,omitempty"`
	Key       string `json:"key,omitempty"`
	Value     string `json:"value,omitempty"`
	Type      string `json:"type,omitempty"`
}

type BuildInfo struct {
	GitHash   string `json:"githash"`
	BuildTime string `json:"buildTime"`
}

type QosParam struct {
	Vol     []Volume `json:"vol,omitempty"`
	Array   string   `json:"array,omitempty"`
	Minbw   uint64   `json:"minbw,omitempty"`
	Maxbw   uint64   `json:"maxbw,omitempty"`
	Miniops uint64   `json:"miniops,omitempty"`
	Maxiops uint64   `json:"maxiops,omitempty"`
}

type Volume struct {
	VolumeName string `json:"volumeName"`
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
