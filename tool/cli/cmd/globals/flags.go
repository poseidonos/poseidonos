package globals

var IsDebug = false
var IsJSONReq = false
var IsJSONRes = false
var IsTestingReqBld = false // True indicates the command is being executed in unit-testing mode.
var DisplayUnit = false     // Display unit (MB, GB, TB, ...) when true
var EnableGrpc = true
var EnableOtel = false

var ReqTimeout uint32 = 180
