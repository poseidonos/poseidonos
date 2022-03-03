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
	UNIQUEID           int      `json:"unique_id"`
	ARRAYNAME          string   `json:"name"`
	STATUS             string   `json:"status,omitempty"`
	STATE              string   `json:"state,omitempty"`
	SITUATION          string   `json:"situation,omitempty"`
	CREATEDATETIME     string   `json:"create_datetime,omitempty"`
	UPDATEDATETIME     string   `json:"update_datetime,omitempty"`
	REBUILDINGPROGRESS int      `json:"rebuilding_progress,omitempty"`
	CAPACITY           uint64   `json:"capacity,omitempty"`
	USED               uint64   `json:"used,omitempty"`
	GCMODE             string   `json:"gc_mode,omitempty"`
	METARAID           string   `json:"meta_raid,omitempty"`
	DATARAID           string   `json:"data_raid,omitempty"`
	DEVICELIST         []Device `json:"devicelist"`
}

type Device struct {
	DEVICENAME string `json:"name,omitempty"`
	DEVICETYPE string `json:"type"`
	ADDRESS    string `json:"addr,omitempty"`
	CLASS      string `json:"class,omitempty"`
	MN         string `json:"mn,omitempty"`
	NUMA       string `json:"numa,omitempty"`
	SIZE       uint64 `json:"size,omitempty"`
	SERIAL     string `json:"sn,omitempty"`
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
	MINORLOGPATH         string `json:"minor_log_path"`
	MAJORLOGPATH         string `json:"major_log_path"`
	LOGFILESIZEINBM      string `json:"logfile_size_in_mb"`
	LOGFILEROTATIONCOUNT int    `json:"logfile_rotation_count"`
	MINALLOWABLELOGLEVEL string `json:"min_allowable_log_level"`
	FILTERENABLED        int    `json:"filter_enabled"`
	FILTERINCLUDED       string `json:"filter_included"`
	FILTEREXCLUDED       string `json:"filter_excluded"`
	STRUCTUREDLOGGING    bool   `json:"structured_logging"`
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
	AVAILABLESPARESPACE     string `json:"available_spare_space"`
	TEMPERATURE             string `json:"temperature"`
	DEVICERELIABILITY       string `json:"device_reliability"`
	READONLY                string `json:"read_only"`
	VOLATILEMEMORYBACKUP    string `json:"volatile_memory_backup"`
	CURRENTTEMPERATURE      string `json:"current_temperature"`
	AVAILABLESPARE          string `json:"available_spare"`
	AVAILABLESPARETHRESHOLD string `json:"available_spare_threshold"`
	LIFEPERCENTAGEUSED      string `json:"life_percentage_used"`
	DATAUNITSREAD           string `json:"data_units_read"`
	DATAUNITSWRITTEN        string `json:"data_units_written"`
	HOSTREADCOMMANDS        string `json:"host_read_commands"`
	HOSTWRITECOMMANDS       string `json:"host_write_commands"`
	CONTROLLERBUSYTIME      string `json:"controller_busy_time"`
	POWERCYCLES             string `json:"power_cycles"`
	POWERONHOURS            string `json:"power_on_hours"`
	UNSAFESHUTDOWNS         string `json:"unsafe_shutdowns"`
	UNRECOVERABLEMEDIAERROS string `json:"unrecoverable_media_errors"`
	LIFETIMEERRORLOGENTRIES string `json:"lifetime_error_log_entries"`
	WARNINGTEMPERATURETIME  string `json:"warning_temperature_time"`
	CRITICALTEMPERATURETIME string `json:"critical_temperature_time"`
	TEMPERATURESENSOR1      string `json:"temperature_sensor1,omitempty"`
	TEMPERATURESENSOR2      string `json:"temperature_sensor2,omitempty"`
	TEMPERATURESENSOR3      string `json:"temperature_sensor3,omitempty"`
	TEMPERATURESENSOR4      string `json:"temperature_sensor4,omitempty"`
	TEMPERATURESENSOR5      string `json:"temperature_sensor5,omitempty"`
	TEMPERATURESENSOR6      string `json:"temperature_sensor6,omitempty"`
	TEMPERATURESENSOR7      string `json:"temperature_sensor7,omitempty"`
	TEMPERATURESENSOR8      string `json:"temperature_sensor8,omitempty"`
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
	VOLUMEID   int    `json:"id,omitempty"`
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
	DEVICELIST []Device `json:"devicelist"`
}

type ListDeviceInfo struct {
	CAPACITY           int    `json:"capacity"`
	REBUILDINGPROGRESS string `json"rebulidingProgress"`
	STATE              string `json:"NOT_EXIST"`
	USED               int    `"json:used"`
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
	NQN             string        `json:"nqn"`
	SUBTYPE         string        `json:"subtype"`
	LISTENADDRESSES []AddressInfo `json:"listen_addresses"`
	ALLOWANYHOST    int           `json:"allow_any_host"`
	HOSTS           []Host        `json:"hosts"`
	SERIAL          string        `json:"serial_number,omitempty"`
	MODEL           string        `json:"model_number,omitempty"`
	MAXNAMESPACES   int           `json:"max_namespaces,omitempty"`
	NAMESPACES      []Namespace   `json:"namespaces,omitempty"`
}

type AddressInfo struct {
	TRANSPORTTYPE      string `json:"transport_type"`
	ADDRESSFAMILY      string `json:"address_family"`
	TARGETADDRESS      string `json:"target_address"`
	TRANSPORTSERVICEID string `json:"transport_service_id"`
}

type Host struct {
	NQN string `json:"nqn"`
}

type Namespace struct {
	NSID     int    `json:"nsid"`
	BDEVNAME string `json:"bdev_name,omitempty"`
	UUID     string `json:"uuid,omitempty"`
}
