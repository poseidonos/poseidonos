package messages

// Request message and param formats of all commands in JSON form.
type Request struct {
	COMMAND   string      `json:"command"`
	RID       string      `json:"rid"`
	PARAM     interface{} `json:"param"`
	REQUESTOR string      `json:"requestor"`
}

func BuildReq(command string, rid string) Request {
	req := Request{
		COMMAND:   command,
		RID:       rid,
		REQUESTOR: "cli",
	}
	return req
}

func BuildReqWithParam(command string, rid string, param interface{}) Request {
	req := Request{
		COMMAND:   command,
		RID:       rid,
		PARAM:     param,
		REQUESTOR: "cli",
	}
	return req
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
	ARRAYNAME string           `json:"name"`
	BUFFER    []DeviceNameList `json:"buffer,omitempty"`
	DATA      []DeviceNameList `json:"data"`
	SPARE     []DeviceNameList `json:"spare,omitempty"`
	RAID      string           `json:"raidtype,omitempty"`
}

type AutocreateArrayParam struct {
	ARRAYNAME    string           `json:"name"`
	BUFFER       []DeviceNameList `json:"buffer,omitempty"`
	NUMDATADEVS  int              `json:"num_data"`
	NUMSPAREDEVS int              `json:"num_spare,omitempty"`
	RAID         string           `json:"raidtype,omitempty"`
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
	ENABLEWT  bool   `json:"enable_write_through,omitempty"`
}

type UnmountArrayParam struct {
	ARRAYNAME string `json:"name"`
}

// Device request messages
type SMARTLOGReqParam struct {
	DEVICENAME string `json:"name"`
}

type CreateDeviceReqParam struct {
	DEVICENAME string `json:"name"`
	NUMBLOCKS  int    `json:"numBlocks"`
	BLOCKSIZE  int    `json:"blockSize"`
	DEVICETYPE string `json:"devType"`
	NUMA       int    `json:"numa"`
}

// Logger request params
type SetLevelReqParam struct {
	LEVEL string `json:"level"`
}

type SetPrefReqParam struct {
	STRUCTUREDLOGGING string `json:"structured_logging,omitempty"`
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
	ISWALVOL     bool   `json:"iswalvol"`
}

type DeleteVolumeParam struct {
	VOLUMENAME string `json:"name"`
	ARRAYNAME  string `json:"array"`
}

type ListVolumeParam struct {
	ARRAYNAME string `json:"array"`
}

type VolumeInfoParam struct {
	ARRAYNAME  string `json:"array"`
	VOLUMENAME string `json:"name"`
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
	MINIOPS      int              `json:"miniops"`
	MAXIOPS      int              `json:"maxiops"`
	MINBANDWIDTH int              `json:"minbw"`
	MAXBANDWIDTH int              `json:"maxbw"`
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
	MAXNAMESPACES int    `json:"maxNamespaces,omitempty"`
	ALLOWANYHOST  bool   `json:"allowAnyHost,omitempty"`
	ANAREPORTING  bool   `json:"anaReporting,omitempty"`
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

type StopRebuildingParam struct {
	ARRAYNAME string `json:"name"`
}

type UpdateEventWrrParam struct {
	NAME   string `json:"name"`
	WEIGHT int    `json:"weight"`
}
