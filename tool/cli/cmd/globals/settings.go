package globals

var (
	IPv4              string = "127.0.0.1"
	Port              string = "18716"
	GrpcPort          string = "50055"
	OtlpPort          string = "30080" 
	GrpcServerAddress string = IPv4 + ":" + GrpcPort
	OtlpServerAddress string = IPv4 + ":" + OtlpPort
	FieldSeparator    string = "|"
	NodeName          string = ""

)

const (
	HaDbIPVar       = "POS_HA_DB_IP_ADDRESS"
	HaDbPortVar     = "POS_HA_DB_PORT"
	HaDbUserVar     = "POS_HA_DB_USERNAME"
	HaDbPasswordVar = "POS_HA_DB_PASSWORD"
	HaDbNameVar     = "POS_HA_DB_NAME"
)
