package messages

// Request message and param formats of all commands in JSON form.
type Request struct {
	COMMAND string      `json:"command"`
	RID     string      `json:"rid"`
	PARAM   interface{} `json:"param"`
}

// Array request params
type SpareParam struct {
	ARRAYNAME string             `json:"array"`
	SPARENAME [1]SpareDeviceName `json:"spare"`
}

type SpareDeviceName struct {
	SPARENAME string `json:"deviceName"`
}

type CreateArrayParam struct {
	ARRAYNAME string            `json:"name"`
	BUFFER    [1]DeviceNameList `json:"buffer"`
	DATA      []DeviceNameList  `json:"data"`
	SPARE     []DeviceNameList  `json:"spare,omitempty"`
	RAID      string            `json:"raidtype,omitempty"`
}

type AutocreateArrayParam struct {
	ARRAYNAME    string            `json:"name"`
	BUFFER       [1]DeviceNameList `json:"buffer"`
	NUMDATADEVS  int               `json:"num_data"`
	NUMSPAREDEVS int               `json:"num_spare,omitempty"`
	RAID         string            `json:"raidtype,omitempty"`
}

type DeviceNameList struct {
	DEVICENAME string `json:"deviceName,omitempty"`
}

type DeleteArrayParam struct {
	ARRAYNAME string `json:"name"`
}

type ListArrayParam struct {
	ARRAYNAME string `json:"name"`
}

type MountArrayParam struct {
	ARRAYNAME string `json:"name"`
}

type UnmountArrayParam struct {
	ARRAYNAME string `json:"name"`
}

// Device request messages
type SMARTReqParam struct {
	DEVICENAME string `json:"name"`
}

type CreateDeviceReqParam struct {
	DEVICENAME string `json:"name"`
	NUMBLOCKS  int    `json:"num_blocks"`
	BLOCKSIZE  int    `json:"block_size"`
	DEVICETYPE string `json:"dev_type"`
	NUMA       int    `json:"numa"`
}

// Loger request params
type SetLevelReqParam struct {
	LEVEL string `json:"level"`
}

// System request params
type SetSystemPropReqParam struct {
	LEVEL string `json:"level"`
}

// Volume request params
type CreateVolumeParam struct {
	VOLUMENAME   string `json:"name"`
	ARRAYNAME    string `json:"array"`
	VOLUMESIZE   uint64 `json:"size"`
	MAXIOPS      int    `json:"maxiops,omitempty"`
	MAXBANDWIDTH int    `json:"maxbw,omitempty"`
}

type DeleteVolumeParam struct {
	VOLUMENAME string `json:"name"`
	ARRAYNAME  string `json:"array"`
}

type ListVolumeParam struct {
	ARRAYNAME string `json:"array"`
}

type MountVolumeParam struct {
	VOLUMENAME string `json:"name"`
	SUBNQN     string `json:"subnqn,omitempty"`
	ARRAYNAME  string `json:"array"`
}

type RenameVolumeParam struct {
	ARRAYNAME     string `json:"array"`
	VOLUMENAME    string `json:"name"`
	NEWVOLUMENAME string `json:"newname"`
}

type UnmountVolumeParam struct {
	VOLUMENAME string `json:"name"`
	ARRAYNAME  string `json:"array"`
}

type VolumeNameList struct {
	VOLUMENAME string `json:"volumeName"`
}

type VolumePolicyParam struct {
	VOLUMENAME   []VolumeNameList `json:"vol"`
	MINIOPS      int              `json:"miniops,omitempty"`
	MAXIOPS      int              `json:"maxiops,omitempty"`
	MINBANDWIDTH int              `json:"minbw,omitempty"`
	MAXBANDWIDTH int              `json:"maxbw,omitempty"`
	ARRAYNAME    string           `json:"array"`
}

type VolumeResetParam struct {
	VOLUMENAME []VolumeNameList `json:"vol"`
	ARRAYNAME  string           `json:"array"`
}

type ListQosParam struct {
	VOLUMENAME []VolumeNameList `json:"vol"`
	ARRAYNAME  string           `json:"array"`
}

// Subsystem request params
type CreateSubsystemParam struct {
	SUBNQN        string `json:"name"`
	SERIAL        string `json:"sn,omitempty"`
	MODEL         string `json:"mn,omitempty"`
	MAXNAMESPACES int    `json:"max_namespaces,omitempty"`
	ALLOWANYHOST  bool   `json:"allow_any_host,omitempty"`
	ANAREPORTING  bool   `json:"ana_reporting,omitempty"`
}

type CreateSubsystemAutoParam struct {
	SUBNQN string `json:"name"`
}

type DeleteSubsystemParam struct {
	SUBNQN string `json:"name"`
}

type ListSubsystemParam struct {
	SUBNQN string `json:"name,omitempty"`
}

type AddListenerParam struct {
	SUBNQN             string `json:"name"`
	TRANSPORTTYPE      string `json:"transport_type"`
	TARGETADDRESS      string `json:"target_address"`
	TRANSPORTSERVICEID string `json:"transport_service_id"`
}

type CreateTransportParam struct {
	TRANSPORTTYPE string `json:"transport_type"`
	BUFCACHESIZE  int    `json:"buf_cache_size,omitempty"`
	NUMSHAREDBUF  int    `json:"num_shared_buf,omitempty"`
}
