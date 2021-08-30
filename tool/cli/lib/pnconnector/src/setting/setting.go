package setting

import (
	"pnconnector/src/log"
	"gopkg.in/yaml.v2"
	"io/ioutil"
	"os"
	"path/filepath"
)

var Config ConfigScheme

type ConfigScheme struct {
	Server           Server `yaml:"server"`
	IBoFOSSocketAddr string
	DAgentSocketAddr string
}

type Server struct {
	Dagent HostConf `yaml:"dagent"`
	IBoF   HostConf `yaml:"ibof"`
	BMC    HostConf `yaml:"bmc"`
}
type HostConf struct {
	IP   string `yaml:"ip"`
	Port string `yaml:"port"`
}

func init() {
	Config.Server.Dagent.IP = "127.0.0.1"
	Config.Server.Dagent.Port = "3000"
	Config.Server.IBoF.IP = "127.0.0.1"
	Config.Server.IBoF.Port = "18716"
	Config.Server.BMC.IP = "192.168.0.2"
	Config.Server.BMC.Port = "443"
}

func LoadConfig() {
	path, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	loadSeverConfig(path + "/config.yaml")
}

func loadSeverConfig(filename string) {
	file, err := ioutil.ReadFile(filename)

	if err != nil {
		log.Infof("LoadSeverConfig : %v\nD-Agent will use default value\n", err)
	} else {
		err = yaml.Unmarshal(file, &Config)
		if err != nil {
			log.Fatalf("loadSeverConfig Error : %v", err)
		} else {
			log.Infof("Open Success : %s", filename)
			log.Infof("Loaded Config Info : %+v", Config)
		}
	}
}