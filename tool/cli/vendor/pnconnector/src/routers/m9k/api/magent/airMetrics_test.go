package magent

import (
	"pnconnector/src/routers/m9k/api/magent/mocks"
	"pnconnector/src/routers/m9k/model"
	"encoding/json"
	"reflect"
	"testing"
)

func TestGetReadBandwidth(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("300"), "time": json.Number("1589872050483860738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("300"), "time": json.Number("1589872050483860738")},
				{"bw": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"bw": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "30d",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"bw": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "5m",
				Level: "0",
			},
			expected: make([]string, 0),
			err:      nil,
		},
		{
			input: model.MAgentParam{
				Time:  "24h",
				Level: "101",
			},
			expected: make([]string, 0),
			err:      nil,
		},
		{
			input: model.MAgentParam{
				Time:  "24h",
				Level: "1",
			},
			expected: make([]string, 0),
			err:      nil,
		},
		{
			input: model.MAgentParam{
				Time:  "20m",
				Level: "0",
			},
			expected: make([]string, 0),
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetReadBandwidth(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}

func TestGetWriteBandwidth(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("300"), "time": json.Number("1589872050483860738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("300"), "time": json.Number("1589872050483860738")},
				{"bw": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"bw": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"bw": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},

		{
			input: model.MAgentParam{
				Time:  "5m",
				Level: "0",
			},
			expected: make([]string, 0),
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetWriteBandwidth(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}

func TestGetReadiops(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("300"), "time": json.Number("1589872050483860738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("300"), "time": json.Number("1589872050483860738")},
				{"iops": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"iops": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},

		{
			input: model.MAgentParam{
				Time:  "5m",
				Level: "0",
			},
			expected: make([]string, 0),
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetReadIOPS(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}

func TestGetWriteiops(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("300"), "time": json.Number("1589872050483860738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("300"), "time": json.Number("1589872050483860738")},
				{"iops": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"iops": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"iops": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},

		{
			input: model.MAgentParam{
				Time:  "5m",
				Level: "0",
			},
			expected: make([]string, 0),
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetWriteIOPS(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}

func TestGetLatency(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"latency": json.Number("300"), "time": json.Number("1589872050483860738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "array",
			},
			expected: []map[string]interface{}{
				{"latency": json.Number("300"), "time": json.Number("1589872050483860738")},
				{"latency": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time:  "15m",
				Level: "0",
			},
			expected: []map[string]interface{}{
				{"latency": json.Number("100"), "time": json.Number("1589872050483860738")},
				{"latency": json.Number("700"), "time": json.Number("1589872050483870738")},
			},
			err: nil,
		},

		{
			input: model.MAgentParam{
				Time: "5m",
			},
			expected: make([]string, 0),
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetLatency(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}
