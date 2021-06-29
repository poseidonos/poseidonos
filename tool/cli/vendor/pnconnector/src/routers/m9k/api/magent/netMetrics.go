package magent

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"encoding/json"
	"fmt"
)

type NetField struct {
	Time        json.Number `json:"time"`
	BytesRecv   json.Number `json:"bytesRecv"`
	BytesSent   json.Number `json:"bytesSent"`
	DropIn      json.Number `json:"dropIn"`
	DropOut     json.Number `json:"dropOut"`
	ErrIn       json.Number `json:"errIn"`
	ErrOut      json.Number `json:"errIn"`
	PacketsRecv json.Number `json:"packetsRecv"`
	PacketsSent json.Number `json:"packetsSent"`
}

type NetFields []NetField

// Getting network data based time parameter and retuning JSON resonse
func GetNetData(param interface{}) (model.Response, error) {
	var res model.Response
	var query string
	fieldsList := make(NetFields, 0)
	paramStruct := param.(model.MAgentParam)

	if paramStruct.Time != "" {
		timeInterval := param.(model.MAgentParam).Time
		if _, found := TimeGroupsDefault[timeInterval]; !found {
			res.Result.Status, _ = util.GetStatusInfo(errEndPointCode)
			res.Result.Data = make([]string, 0)
			return res, nil
		}
		if Contains(AggTime, timeInterval) {
			query = fmt.Sprintf(netAggRPQ, DBName, AggRP, timeInterval)
		} else {
			query = fmt.Sprintf(netDefaultRPQ, DBName, DefaultRP, timeInterval, TimeGroupsDefault[timeInterval])
		}
	} else {
		query = fmt.Sprintf(netLastRecordQ, DBName, DefaultRP)
	}
	result, err := ExecuteQuery(query)

	if err != nil {
		res.Result.Status, _ = util.GetStatusInfo(errQueryCode)
		res.Result.Data = make([]string, 0)
		return res, nil
	}

	if len(result) == 0 || len(result[0].Series) == 0 {
		res.Result.Status, _ = util.GetStatusInfo(errDataCode)
		res.Result.Data = make([]string, 0)
		return res, nil
	}

	for _, values := range result[0].Series[0].Values {
		if values[1] != nil {
			fieldsList = append(fieldsList, NetField{values[0].(json.Number), values[1].(json.Number), values[2].(json.Number), values[3].(json.Number), values[4].(json.Number), values[5].(json.Number), values[6].(json.Number), values[7].(json.Number), values[8].(json.Number)})
		}
	}
	res.Result.Status, _ = util.GetStatusInfo(0)
	res.Result.Data = fieldsList

	return res, nil
}
