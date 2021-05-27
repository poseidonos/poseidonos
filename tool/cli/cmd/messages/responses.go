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
	DESCRIPTION string `json:"description"`
}

type Info struct {
	VERSION string `json:"version"`
}

// Response for LISTARRAY Command
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

type Array struct {
	ARRAYNAME      string   `json:"name"`
	CREATEDATETIME string   `json:"createDatetime"`
	STATUS         string   `json:"status"`
	UPDATEDATETIME string   `json:"updateDatetime"`
	DEVICELIST     []Device `json:"devicelist"`
}

type Device struct {
	DEVICENAME string `json:"name"`
	DEVICETYPE string `json:"type"`
	ADDRESS    string `json:"addr,omitempty"`
	CLASS      string `json:"class,omitempty"`
	MN         string `json:"mn,omitempty"`
	NUMA       string `json:"numa,omitempty"`
	SIZE       int    `json:"size,omitempty"`
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
	MINORLOGPATH                   string `json:"minor_log_path"`
	MAJORLOGPATH                   string `json:"major_log_path"`
	LOGFILESIZEINBM                string `json:"logfile_size_in_mb"`
	LOGFILEROTATIONCOUNT           int    `json:"logfile_rotation_count"`
	MINALLOWABLELOGLEVEL           string `json:"min_allowable_log_level"`
	DEDUPLICATIONENABLED           bool   `json:"deduplication_enabled"`
	DEDUPLICATIONSENSITIVITYINMSEC int    `json:"deduplication_sensitivity_in_msec"`
	FILTERENABLED                  bool   `json:"filter_enabled"`
	FILTERINCLUDED                 string `json:"filter_included"`
	FILTEREXCLUDED                 string `json:"filter_excluded"`
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
type SMARTResponse struct {
	RID     string      `json:"rid"`
	COMMAND string      `json:"command"`
	RESULT  SMARTResult `json:"result,omitempty"`
	INFO    Info        `json:"info"`
}

type SMARTResult struct {
	STATUS Status       `json:"status,omitempty"`
	DATA   SMARTResData `json:"data,omitempty"`
}

type SMARTResData struct {
	PERCENTAGEUSED string `json:"percentage_used"`
	TEMPERATURE    string `json:"temperature"`
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
	VOLUMENAME string `json:"array"`
	VOLUMEID   string `json:"id"`
	TOTAL      int    `json:"total"`
	REMAIN     int    `json:"remain"`
	STATUS     string `json:"status"`
	MAXIOPS    int    `json:"maxiops"`
	MAXBW      int    `json:"maxbw"`
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
