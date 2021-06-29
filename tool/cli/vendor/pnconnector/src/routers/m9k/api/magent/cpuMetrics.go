package magent

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"encoding/json"
	"fmt"
)

// Contains function chcks if a string is present in an array of strings
func Contains(arr []string, element string) bool {
	for _, n := range arr {
		if element == n {
			return true
		}
	}
	return false
}

// CPUField stores the cpu response structure
type CPUField struct {
	Time      json.Number `json:"time"`
	UsageUser json.Number `json:"cpuUsagePercent"`
}

// CPUFields is an array of CPUField
type CPUFields []CPUField

// GetCPUData is for getting CPU data based time parameter and retuning JSON resonse
func GetCPUData(param interface{}) (model.Response, error) {
	var res model.Response
	var query string
	fieldsList := make(CPUFields, 0)
	paramStruct := param.(model.MAgentParam)

	if paramStruct.Time != "" {
		timeInterval := param.(model.MAgentParam).Time
		if _, found := TimeGroupsDefault[timeInterval]; !found {
			res.Result.Status, _ = util.GetStatusInfo(errEndPointCode)
			res.Result.Data = make([]string, 0)
			return res, nil
		}
		if Contains(AggTime, timeInterval) {
			query = fmt.Sprintf(cpuAggRPQ, DBName, AggRP, timeInterval)
		} else {
			query = fmt.Sprintf(cpuDefaultRPQ, DBName, DefaultRP, timeInterval, TimeGroupsDefault[timeInterval])
		}
	} else {
		query = fmt.Sprintf(cpuLastRecordQ, DBName, DefaultRP)
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
			fieldsList = append(fieldsList, CPUField{values[0].(json.Number), values[1].(json.Number)})
		}
	}
	res.Result.Status, _ = util.GetStatusInfo(0)
	res.Result.Data = fieldsList
	return res, nil
}
