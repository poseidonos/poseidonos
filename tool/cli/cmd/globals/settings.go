package globals

var IPv4 string = "127.0.0.1"
var Port string = "18716"
var GrpcPort string = "50055"
var GrpcServerAddress string = IPv4 + ":" + GrpcPort
var FieldSeparator = "|"

const (
	HaDbIPVar       = "POS_HA_DB_IP_ADDRESS"
	HaDbPortVar     = "POS_HA_DB_PORT"
	HaDbUserVar     = "POS_HA_DB_USERNAME"
	HaDbPasswordVar = "POS_HA_DB_PASSWORD"
	HaDbNameVar     = "POS_HA_DB_NAME"
)
