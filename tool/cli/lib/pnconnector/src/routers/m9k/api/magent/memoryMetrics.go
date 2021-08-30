package magent

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"encoding/json"
	"fmt"
)

type MemoryField struct {
	Time      json.Number `json:"time"`
	UsageUser json.Number `json:"memoryUsagePercent"`
}

type MemoryFields []MemoryField

// Getting Memory data based time parameter and retuning JSON resonse
func GetMemoryData(param interface{}) (model.Response, error) {
	var res model.Response
	var query string
	fieldsList := make(MemoryFields, 0)
	paramStruct := param.(model.MAgentParam)

	if paramStruct.Time != "" {
		timeInterval := param.(model.MAgentParam).Time
		if _, found := TimeGroupsDefault[timeInterval]; !found {
			res.Result.Status, _ = util.GetStatusInfo(errEndPointCode)
			res.Result.Data = make([]string, 0)
			return res, nil
		}
		if Contains(AggTime, timeInterval) {
			query = fmt.Sprintf(memoryAggRPQ, DBName, AggRP, timeInterval)
		} else {
			query = fmt.Sprintf(memoryDefaultRPQ, DBName, DefaultRP, timeInterval, TimeGroupsDefault[timeInterval])
		}
	} else {
		query = fmt.Sprintf(memoryLastRecordQ, DBName, DefaultRP)
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
			fieldsList = append(fieldsList, MemoryField{values[0].(json.Number), values[1].(json.Number)})
		}
	}

	res.Result.Status, _ = util.GetStatusInfo(0)
	res.Result.Data = fieldsList
	return res, nil
}
