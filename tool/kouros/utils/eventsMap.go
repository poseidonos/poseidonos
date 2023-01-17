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
package utils

import (
	_ "fmt"
	"gopkg.in/yaml.v2"
	"kouros/errors"
	"kouros/log"
	"kouros/model"
)

var eventsmap PosEvents

type info2 struct {
	Code     int    `yaml:"code"`
	Level    string `yaml:"level"`
	Message  string `yaml:"message"`
	Problem  string `yaml:"problem,omitempty"`
	Solution string `yaml:"solution,omitempty"`
}

type module struct {
	Name    string  `yaml:"name"`
	Count   int     `yaml:"count"`
	Idstart int     `yaml:"idStart"`
	Idend   int     `yaml:"idEnd"`
	Info    []info2 `yaml:"info"`
}
type PosEvents struct {
	Modules []module `yaml:"modules"`
}

func init() {
	LoadEvents()
}

func LoadEvents() {
	file, err := Asset("../resources/events.yaml")

	if err != nil {
		log.Infof("LoadSeverConfig : %v\n EventId cannot be decoded\n", err)
	} else {
		err = yaml.Unmarshal(file, &eventsmap)
		if err != nil {
			log.Fatalf("loadevents Error : %v", err)
		}
	}
}

func GetStatusInfo(code int) (model.Status, error) {
	var status model.Status
	status.Code = code
	totMods := len(eventsmap.Modules)
	for i := 0; i < totMods; i++ {
		if code >= eventsmap.Modules[i].Idstart && code <= eventsmap.Modules[i].Idend {
			totInfo := len(eventsmap.Modules[i].Info)

			for j := 0; j < totInfo; j++ {
				if eventsmap.Modules[i].Info[j].Code == code {
					status.Module = eventsmap.Modules[i].Name
					status.Description = eventsmap.Modules[i].Info[j].Message
					status.Problem = eventsmap.Modules[i].Info[j].Problem
					status.Solution = eventsmap.Modules[i].Info[j].Solution
					status.Level = eventsmap.Modules[i].Info[j].Level

					return status, nil
				}
			}
		}
	}

	err := errors.New("there is no event info")

	return status, err
}
