/*In this code we are using gin framework and influxdb golang client libraries
Gin is a web framework written in Go (Golang). It features a martini-like API with performance that is up to 40 times faster than other frameworks


DESCRIPTION: This file contains the code for AIR related data from influxdb
NAME : airMetrics.go
@AUTHORS: Aswin K K
@Version : 1.0 *
@REVISION HISTORY
[6/26/2020] [aswin.kk] : Code for Bandwidth, IOPS and Latency Added

*/

package magent

import (
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"encoding/json"
	"fmt"
	"regexp"
	"strconv"
)

// GetAIRData fetches AIR data from influx db based on time parameter and returns the values and fields
func GetAIRData(param interface{}, AggRPQ, DefaultRPQ, LastRecordQ, startingTime string) ([][]interface{}, []string, int) {
	var query string
	paramStruct := param.(model.MAgentParam)
	if paramStruct.Time != "" {
		timeInterval := param.(model.MAgentParam).Time
		if _, found := TimeGroupsDefault[timeInterval]; !found {
			return nil, nil, errEndPointCode
		}
		if Contains(AggTime, timeInterval) {
			query = fmt.Sprintf(AggRPQ, DBName, AggRP, timeInterval, startingTime)
		} else {
			query = fmt.Sprintf(DefaultRPQ, DBName, DefaultRP, timeInterval, startingTime, TimeGroupsDefault[timeInterval])
		}

	} else {
		query = fmt.Sprintf(LastRecordQ, DBName, DefaultRP)
	}
	result, err := ExecuteQuery(query)
	if err != nil {
		return nil, nil, errQueryCode
	}

	if len(result) == 0 || len(result[0].Series) == 0 {
		return nil, nil, errDataCode
	}
	return result[0].Series[0].Values, result[0].Series[0].Columns, 0
}

// extractValues contains the parsing logic for extracting array level and volume level data
func extractValues(values [][]interface{}, columns []string, key, metrics, metricOps, level string) []map[string]interface{} {
	result := []map[string]interface{}{}
	valueIndexes := []int{}
	if level == "array" {
		for index, col := range columns {
			if match, _ := regexp.MatchString(".*aid$", col); !match && col != "time" && col != "timestamp" {
				valueIndexes = append(valueIndexes, index)
			}
		}
		for _, val := range values {
			currentValue := make(map[string]interface{})
			currentValue["time"] = val[0]
			sum := 0.0
			for _, valIndex := range valueIndexes {
				if val[valIndex] != nil {
					v, _ := val[valIndex].(json.Number).Float64()
					sum += v
				}
			}
			currentValue[key] = json.Number(strconv.FormatFloat(sum, 'f', -1, 64))
			result = append(result, currentValue)
		}
	} else {
		indexMap := make(map[string]int)
		aidArr := []string{}
		volID, err := strconv.Atoi(level)
		if err != nil {
			return result
		}
		for index, col := range columns {
			indexMap[col] = index
			if match, _ := regexp.MatchString(".*aid$", col); match {
				aidArr = append(aidArr, col)
			}
		}
		for _, val := range values {
			currentValue := make(map[string]interface{})
			currentValue["time"] = val[0]
			sum := 0.0
			for _, aid := range aidArr {
				if _, ok := indexMap[aid]; ok && val[indexMap[aid]] != nil {
					if idx, err := val[indexMap[aid]].(json.Number).Int64(); err == nil && idx == int64(volID) {
						fieldKey := aid[:(len(aid)-len("aid"))] + metricOps
						if len(aid) > 4 && aid[:4] != MeanFieldKey && aid[:4] != PerfFieldKey && aid[:4] != LatFieldKey {
							fieldKey = MeanFieldKey + aid[len(metrics):(len(aid)-len("aid"))] + metricOps
						}
						if _, ok = indexMap[fieldKey]; ok && val[indexMap[fieldKey]] != nil {
							v, _ := val[indexMap[fieldKey]].(json.Number).Float64()
							sum += v
						}
					}
				}
			}
			currentValue[key] = json.Number(strconv.FormatFloat(sum, 'f', -1, 64))
			result = append(result, currentValue)
		}
	}
	return result
}

func getVolumeCreationTime(level string) string {
	volQuery := fmt.Sprintf(VolumeQuery, DBName, DefaultRP, level)
	result, err := ExecuteQuery(volQuery)
	if err != nil || len(result) == 0 || len(result[0].Series) == 0 {
		return "0"
	}
	for index, column := range result[0].Series[0].Columns {
		if column == "time" {
			return string(result[0].Series[0].Values[index][0].(json.Number))
		}
	}
	return "0"
}

// GetReadBandwidth returns metrics related to Read Bandwidth
func GetReadBandwidth(param interface{}) (model.Response, error) {
	var result model.Response
	level := param.(model.MAgentParam).Level
	startingTime := getVolumeCreationTime(level)
	values, columns, statusCode := GetAIRData(param, ReadBandwidthAggRPQ, ReadBandwidthDefaultRPQ, ReadBandwidthLastRecordQ, startingTime)
	result.Result.Status, _ = util.GetStatusInfo(statusCode)
	if statusCode != 0 {
		result.Result.Data = make([]string, 0)
		return result, nil
	}
	res := extractValues(values, columns, "bw", "bw", BWReadField, level)
	result.Result.Data = res
	return result, nil
}

// GetWriteBandwidth returns the metrics related to Write Bandwidth
func GetWriteBandwidth(param interface{}) (model.Response, error) {
	var result model.Response
	level := param.(model.MAgentParam).Level
	startingTime := getVolumeCreationTime(level)
	values, columns, statusCode := GetAIRData(param, WriteBandwidthAggRPQ, WriteBandwidthDefaultRPQ, WriteBandwidthLastRecordQ, startingTime)
	result.Result.Status, _ = util.GetStatusInfo(statusCode)
	if statusCode != 0 {
		result.Result.Data = make([]string, 0)
		return result, nil
	}
	res := extractValues(values, columns, "bw", "bw", BWWriteField, level)
	result.Result.Data = res
	return result, nil
}

// GetReadIOPS returns the metrics related to Read IOPS
func GetReadIOPS(param interface{}) (model.Response, error) {
	var result model.Response
	level := param.(model.MAgentParam).Level
	startingTime := getVolumeCreationTime(level)
	values, columns, statusCode := GetAIRData(param, ReadIOPSAggRPQ, ReadIOPSDefaultRPQ, ReadIOPSLastRecordQ, startingTime)
	result.Result.Status, _ = util.GetStatusInfo(statusCode)
	if statusCode != 0 {
		result.Result.Data = make([]string, 0)
		return result, nil
	}
	res := extractValues(values, columns, "iops", "iops", IOPSReadField, level)
	result.Result.Data = res
	return result, nil
}

// GetWriteIOPS returns the metrics related to Write IOPS
func GetWriteIOPS(param interface{}) (model.Response, error) {
	var result model.Response
	level := param.(model.MAgentParam).Level
	startingTime := getVolumeCreationTime(level)
	values, columns, statusCode := GetAIRData(param, WriteIOPSAggRPQ, WriteIOPSDefaultRPQ, WriteIOPSLastRecordQ, startingTime)
	result.Result.Status, _ = util.GetStatusInfo(statusCode)
	if statusCode != 0 {
		result.Result.Data = make([]string, 0)
		result.Result.Status, _ = util.GetStatusInfo(statusCode)
		return result, nil
	}
	res := extractValues(values, columns, "iops", "iops", IOPSWriteField, level)
	result.Result.Data = res
	return result, nil
}

// GetLatency collects the latency metrics from influxdb
func GetLatency(param interface{}) (model.Response, error) {
	var result model.Response
	level := param.(model.MAgentParam).Level
	startingTime := getVolumeCreationTime(level)
	values, columns, statusCode := GetAIRData(param, LatencyAggRPQ, LatencyDefaultRPQ, LatencyLastRecordQ, startingTime)
	result.Result.Status, _ = util.GetStatusInfo(statusCode)
	if statusCode != 0 {
		result.Result.Data = make([]string, 0)
		return result, nil
	}
	res := extractValues(values, columns, "latency", "latency", LatencyField, level)
	result.Result.Data = res
	return result, nil
}
