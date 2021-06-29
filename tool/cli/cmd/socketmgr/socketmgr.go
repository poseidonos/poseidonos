package socketmgr

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"net"
	"path/filepath"
	"pnconnector/src/log"

	"gopkg.in/yaml.v2"
)

type structServerConfig struct {
	IPAddress string `yaml:"ServerIPAddress"`
	Port      string `yaml:"ServerPort"`
}

var conn net.Conn
var ServerConfig structServerConfig

func Connect() {

	filename, err := filepath.Abs("socketmgr/setting.yaml")
	yamlFile, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Debug("error:", err)
		log.Debug("Using default socket address: localhost:18716")

		ServerConfig.IPAddress = "127.0.0.1"
		ServerConfig.Port = "18716"
	}

	err = yaml.Unmarshal(yamlFile, &ServerConfig)
	if err != nil {
		log.Debug("error:", err)
	}

	// Connect to server
	conn, err = net.Dial("tcp", ServerConfig.IPAddress+":"+ServerConfig.Port)
	if err != nil {
		log.Debug("error:", err)
	}
}

func SendReqAndReceiveRes(reqJSON string) string {

	fmt.Fprintf(conn, reqJSON)
	// wait for reply
	res, _ := bufio.NewReader(conn).ReadString('\n')

	return res
}

func Close() {
	conn.Close()
}
