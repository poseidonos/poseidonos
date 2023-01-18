/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package setting

import (
	"gopkg.in/yaml.v2"
	"io/ioutil"
	"kouros/log"
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
	IP                 string `yaml:"ip"`
	Port               string `yaml:"port"`
	GrpcPort           string `yaml:"grpc_port"`
	TargetAddress      string `yaml:"target_address"`
	TransportType      string `yaml:"transport_type"`
	BufCacheSize       int    `yaml:"buf_cache_size"`
	NumSharedBuf       int    `yaml:"num_shared_buf"`
	Subsystem_1        string `yaml:"subsystem_1"`
	Subsystem_2        string `yaml:"subsystem_2"`
	TransportServiceId string `yaml:"transport_service_id"`
	Serial             string `yaml:"sn"`
	Model              string `yaml:"mn"`
	MaxNameSpaces      int    `yaml:"max_namespaces"`
	AllowAnyHost       bool   `yaml:"allow_any_host"`
	Uram1              string `yaml:"uram1"`
	Uram2              string `yaml:"uram2"`
	NumBlocks          int    `yaml:"num_blocks"`
	BlockSize          int    `yaml:"block_size"`
	DevType            string `yaml:"dev_type"`
	Numa               int    `yaml:"numa"`
	AnaReporting       bool   `yaml:"ana_reporting"`
}

func init() {
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
