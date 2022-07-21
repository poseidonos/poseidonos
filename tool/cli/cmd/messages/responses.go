package messages

// JSON message formats for ADDSPARE command
type Response struct {
	RID     string `json:"rid"`
	COMMAND string `json:"command"`
	RESULT  Result `json:"result,omitempty"`
	INFO    Info   `json:"info"`
}

type Result struct {
	STATUS Status      `json:"status,omitempty"`
	DATA   interface{} `json:"data,omitempty"`
}

type Status struct {
	CODE        int    `json:"code"`
	EVENTNAME   string `json:"eventName"`
	DESCRIPTION string `json:"description"`
	CAUSE       string `json:"cause"`
	SOLUTION    string `json:"solution"`
}

type Info struct {
	VERSION string `json:"version"`
}

// Respons for SYSTEM commands

type POSInfoResponse struct {
	RID     string        `json:"rid"`
	COMMAND string        `json:"command"`
	RESULT  POSInfoResult `json:"result,omitempty"`
	INFO    Info          `json:"info"`
}

type POSInfoResult struct {
	STATUS Status      `json:"status,omitempty"`
	DATA   POSInfoData `json:"data,omitempty"`
}

type POSInfoData struct {
	VERSION string `json:version"`
}

type POSPropertyResponse struct {
	RID     string            `json:"rid"`
	COMMAND string            `json:"command"`
	RESULT  POSPropertyResult `json:"result,omitempty"`
	INFO    Info              `json:"info"`
}

type POSPropertyResult struct {
	STATUS Status          `json:"status,omitempty"`
	DATA   POSPropertyData `json:"data,omitempty"`
}

type POSPropertyData struct {
	REBUILDPOLICY string `json:"rebuildPolicy"`
}

// Response for LISTARRAY & ARRAYINFO commands
type ListArrayResponse struct {
	RID     string          `json:"rid"`
	COMMAND string          `json:"command"`
	RESULT  ListArrayResult `json:"result,omitempty"`
	INFO    Info            `json:"info"`
}

type ListArrayResult struct {
	STATUS Status           `json:"status,omitempty"`
	DATA   ListArrayResData `json:"data,omitempty"`
}

type ListArrayResData struct {
	ARRAYLIST []Array `json:"arrayList"`
}

type ArrayInfoResponse struct {
	RID     string          `json:"rid"`
	COMMAND string          `json:"command"`
	RESULT  ArrayInfoResult `json:"result,omitempty"`
	INFO    Info            `json:"info"`
}

type ArrayInfoResult struct {
	STATUS Status `json:"status,omitempty"`
	DATA   Array  `json:"data,omitempty"`
}

// TODO(mj): use omitempty because LISTARRAY and ARRAYINFO have different array structure.
// The commands will be merged and this should be revised.
type Array struct {
	ARRAYINDEX         int      `json:"index"`
	UNIQUEID           int      `json:"uniqueId"`
	ARRAYNAME          string   `json:"name"`
	STATUS             string   `json:"status,omitempty"`
	STATE              string   `json:"state,omitempty"`
	SITUATION          string   `json:"situation,omitempty"`
	CREATEDATETIME     string   `json:"createDatetime,omitempty"`
	UPDATEDATETIME     string   `json:"updateDatetime,omitempty"`
	REBUILDINGPROGRESS uint32   `json:"rebuildingProgress,omitempty"`
	CAPACITY           string   `json:"capacity,omitempty"`
	USED               string   `json:"used,omitempty"`
	GCMODE             string   `json:"gcMode,omitempty"`
	METARAID           string   `json:"metaRaid,omitempty"`
	DATARAID           string   `json:"dataRaid,omitempty"`
	WRITETHROUGH       bool     `json:"writeThroughEnabled,omitempty"`
	DEVICELIST         []Device `json:"devicelist"`
}

type Device struct {
	DEVICENAME string `json:"name,omitempty"`
	DEVICETYPE string `json:"type"`
	ADDRESS    string `json:"addr,omitempty"`
	CLASS      string `json:"class,omitempty"`
	MN         string `json:"modelNumber,omitempty"`
	NUMA       string `json:"numa,omitempty"`
	SIZE       uint64 `json:"size,omitempty"`
	SERIAL     string `json:"serialNumber,omitempty"`
}

// Response for LOGGERINFO command
type LoggerInfoResponse struct {
	RID     string           `json:"rid"`
	COMMAND string           `json:"command"`
	RESULT  LoggerInfoResult `json:"result,omitempty"`
	INFO    Info             `json:"info"`
}

type LoggerInfoResult struct {
	STATUS Status            `json:"status,omitempty"`
	DATA   LoggerInfoResData `json:"data,omitempty"`
}

type LoggerInfoResData struct {
	MINORLOGPATH         string `json:"minorLogPath"`
	MAJORLOGPATH         string `json:"majorLogPath"`
	LOGFILESIZEINBM      string `json:"logfileSizeInMb"`
	LOGFILEROTATIONCOUNT int    `json:"logfileRotationCount"`
	MINALLOWABLELOGLEVEL string `json:"minAllowableLogLevel"`
	FILTERENABLED        int    `json:"filterEnabled"`
	FILTERINCLUDED       string `json:"filterIncluded"`
	FILTEREXCLUDED       string `json:"filterExcluded"`
	STRUCTUREDLOGGING    bool   `json:"structuredLogging"`
}

// Response for GETLOGLEVEL command
type GetLogLevelResponse struct {
	RID     string            `json:"rid"`
	COMMAND string            `json:"command"`
	RESULT  GetLogLevelResult `json:"result,omitempty"`
	INFO    Info              `json:"info"`
}

type GetLogLevelResult struct {
	STATUS Status             `json:"status,omitempty"`
	DATA   GetLogLevelResData `json:"data,omitempty"`
}

type GetLogLevelResData struct {
	LEVEL string `json:level"`
}

// Response for SMART command
type SMARTLOGResponse struct {
	RID     string         `json:"rid"`
	COMMAND string         `json:"command"`
	RESULT  SMARTLOGResult `json:"result,omitempty"`
	INFO    Info           `json:"info"`
}

type SMARTLOGResult struct {
	STATUS Status   `json:"status,omitempty"`
	DATA   SMARTLog `json:"data,omitempty"`
}

type SMARTLog struct {
	AVAILABLESPARESPACE     string `json:"availableSpareSpace"`
	TEMPERATURE             string `json:"temperature"`
	DEVICERELIABILITY       string `json:"deviceReliability"`
	READONLY                string `json:"readOnly"`
	VOLATILEMEMORYBACKUP    string `json:"volatileMemoryBackup"`
	CURRENTTEMPERATURE      string `json:"currentTemperature"`
	AVAILABLESPARE          string `json:"availableSpare"`
	AVAILABLESPARETHRESHOLD string `json:"availableSpareThreshold"`
	LIFEPERCENTAGEUSED      string `json:"lifePercentageUsed"`
	DATAUNITSREAD           string `json:"dataUnitsRead"`
	DATAUNITSWRITTEN        string `json:"dataUnitsWritten"`
	HOSTREADCOMMANDS        string `json:"hostReadCommands"`
	HOSTWRITECOMMANDS       string `json:"hostWriteCommands"`
	CONTROLLERBUSYTIME      string `json:"controllerBusyTime"`
	POWERCYCLES             string `json:"powerCycles"`
	POWERONHOURS            string `json:"powerOnHours"`
	UNSAFESHUTDOWNS         string `json:"unsafeShutdowns"`
	UNRECOVERABLEMEDIAERROS string `json:"unrecoverableMediaErrors"`
	LIFETIMEERRORLOGENTRIES string `json:"lifetimeErrorLogEntries"`
	WARNINGTEMPERATURETIME  string `json:"warningTemperatureTime"`
	CRITICALTEMPERATURETIME string `json:"criticalTemperatureTime"`
	TEMPERATURESENSOR1      string `json:"temperatureSensor1,omitempty"`
	TEMPERATURESENSOR2      string `json:"temperatureSensor2,omitempty"`
	TEMPERATURESENSOR3      string `json:"temperatureSensor3,omitempty"`
	TEMPERATURESENSOR4      string `json:"temperatureSensor4,omitempty"`
	TEMPERATURESENSOR5      string `json:"temperatureSensor5,omitempty"`
	TEMPERATURESENSOR6      string `json:"temperatureSensor6,omitempty"`
	TEMPERATURESENSOR7      string `json:"temperatureSensor7,omitempty"`
	TEMPERATURESENSOR8      string `json:"temperatureSensor8,omitempty"`
}

// Response for VOLUMEINFO command
type VolumeInfoResponse struct {
	RID     string           `json:"rid"`
	COMMAND string           `json:"command"`
	RESULT  VolumeInfoResult `json:"result,omitempty"`
	INFO    Info             `json:"info,omitempty"`
}

type VolumeInfoResult struct {
	STATUS Status `json:"status,omitempty"`
	DATA   Volume `json:"data,omitempty"`
}

// Response for LISTVOLUME command
type ListVolumeResponse struct {
	RID     string           `json:"rid"`
	COMMAND string           `json:"command"`
	RESULT  ListVolumeResult `json:"result,omitempty"`
	INFO    Info             `json:"info,omitempty"`
}

type ListVolumeResult struct {
	STATUS Status            `json:"status,omitempty"`
	DATA   ListVolumeResData `json:"data,omitempty"`
}

type ListVolumeResData struct {
	ARRAYNAME  string   `json:"array"`
	VOLUMELIST []Volume `json:"volumes"`
}

type Volume struct {
	VOLUMENAME string `json:"name"`
	INDEX      int    `json:"index,omitempty"`
	TOTAL      uint64 `json:"total"`
	REMAIN     uint64 `json:"remain"`
	STATUS     string `json:"status"`
	MAXIOPS    int    `json:"maxiops"`
	MAXBW      int    `json:"maxbw"`
	MINIOPS    int    `json:"miniops"`
	MINBW      int    `json:"minbw"`
	SUBNQN     string `json:"subnqn,omitempty"`
	UUID       string `json:"uuid,omitempty"`
	ARRAYNAME  string `json:"array_name,omitempty"`
}

// Response for LISTDEVICE Command
type ListDeviceResponse struct {
	RID             string           `json:"rid"`
	LASTSUCCESSTIME string           `json:"lastSuccessTime"`
	RESULT          ListDeviceResult `json:"result,omitempty"`
	INFO            Info             `json:"info"`
}

type ListDeviceResult struct {
	STATUS Status            `json:"status,omitempty"`
	DATA   ListDeviceResData `json:"data,omitempty"`
	INFO   ListDeviceInfo    `json:"info,omitempty"`
}

type ListDeviceResData struct {
	DEVICELIST []Device `json:"deviceList"`
}

type ListDeviceInfo struct {
	CAPACITY           int    `json:"capacity"`
	REBUILDINGPROGRESS string `json:"rebulidingProgress"`
	STATE              string `json:"NOT_EXIST"`
	USED               int    `json:"used"`
}

type ListQosResponse struct {
	RID     string    `json:"rid"`
	COMMAND string    `json:"command"`
	RESULT  QosResult `json:"result"`
	INFO    Info      `json:"info"`
}

type QosResult struct {
	STATUS Status        `json:"status"`
	DATA   ListQosResult `json:"data,omitempty"`
}

type ListQosResult struct {
	QOSARRAYNAME  []ArrayNameQos `json:"arrayName"`
	REBUILDPOLICY []Rebuild      `json:"rebuildPolicy,omitempty"`
	VOLUMEQOSLIST []VolumeQos    `json:"volumePolicies"`
}

type Rebuild struct {
	IMPACT string `json:"rebuild,omitempty"`
}

type ArrayNameQos struct {
	ARRNAME string `json:"ArrayName"`
}

type VolumeQos struct {
	VOLUMENAME       string `json:"name"`
	VOLUMEID         int    `json:"id"`
	MINIOPS          int    `json:"miniops"`
	MAXIOPS          int    `json:"maxiops"`
	MINBW            int    `json:"minbw"`
	MAXBW            int    `json:"maxbw"`
	MINBWGUARANTEE   string `json:"min_bw_guarantee"`
	MINIOPSGUARANTEE string `json:"min_iops_guarantee"`
}

// Response for LISTSUBSYSTEM Command
type ListSubsystemResponse struct {
	RID     string              `json:"rid"`
	COMMAND string              `json:"command"`
	RESULT  ListSubsystemResult `json:"result,omitempty"`
	INFO    Info                `json:"info"`
}

type ListSubsystemResult struct {
	STATUS Status               `json:"status,omitempty"`
	DATA   ListSubsystemResData `json:"data,omitempty"`
}

type ListSubsystemResData struct {
	SUBSYSTEMLIST []Subsystem `json:"subsystemlist"`
}

type Subsystem struct {
	NQN             string        `json:"subnqn"`
	SUBTYPE         string        `json:"subtype"`
	LISTENADDRESSES []AddressInfo `json:"listenAddresses"`
	ALLOWANYHOST    int           `json:"allowAnyHost"`
	HOSTS           []Host        `json:"hosts"`
	SERIAL          string        `json:"serialNumber,omitempty"`
	MODEL           string        `json:"modelNumber,omitempty"`
	MAXNAMESPACES   int           `json:"maxNamespaces,omitempty"`
	NAMESPACES      []Namespace   `json:"namespaces,omitempty"`
}

type AddressInfo struct {
	TRANSPORTTYPE      string `json:"transportType"`
	ADDRESSFAMILY      string `json:"addressFamily"`
	TARGETADDRESS      string `json:"targetAddress"`
	TRANSPORTSERVICEID string `json:"transportServiceId"`
}

type Host struct {
	NQN string `json:"nqn"`
}

type Namespace struct {
	NSID     int    `json:"nsid"`
	BDEVNAME string `json:"bdevName,omitempty"`
	UUID     string `json:"uuid,omitempty"`
}
