package messages

// Request message and param formats of all commands in JSON form.
type Request struct {
	COMMAND string      `json:"command"`
	RID     string      `json:"rid"`
	PARAM   interface{} `json:"param,omitempty"`
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
	RAID      string            `json:"raidtype"`
	BUFFER    [1]DeviceNameList `json:"buffer"`
	DATA      []DeviceNameList  `json:"data"`
	SPARE     [1]DeviceNameList `json:"spare"`
}

type DeviceNameList struct {
	DEVICENAME string `json:"deviceName"`
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
	VOLUMESIZE   int    `json:"size"`
	MAXIOPS      int    `json:"maxiops,omitempty"`
	MAXBANDWIDTH int    `json:"maxbw,omitempty"`
	ARRAYNAME    string `json:"array"`
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

type SetPropertyVolumeParam struct {
	VOLUMENAME   string `json:"name"`
	MAXIOPS      int    `json:"maxiops,omitempty"`
	MAXBANDWIDTH int    `json:"maxbw,omitempty"`
	ARRAYNAME    string `json:"array"`
}

// Subsystem request params
type DeleteSubsystemParam struct {
	SUBNQN string `json:"name"`
}
