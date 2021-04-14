package magent

import (
	"pnconnector/src/routers/m9k/api/magent/mocks"
	"pnconnector/src/routers/m9k/model"
	"encoding/json"
	"reflect"
	"testing"
)

func TestGetCPUData(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{},
			expected: CPUFields{
				{
					UsageUser: json.Number("20"),
					Time:      json.Number("1589872050483860738"),
				},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time: "15m",
			},
			expected: CPUFields{
				{
					UsageUser: json.Number("20"),
					Time:      json.Number("1589872050483860738"),
				},
				{
					UsageUser: json.Number("30"),
					Time:      json.Number("1589872050483870738"),
				},
			},
			err: nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetCPUData(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}

}

func TestGetCPUDataError(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected int
		err      error
	}{
		{
			input: model.MAgentParam{
				Time: "115m", //incorrect param
			},
			expected: 21010,
			err:      nil,
		},
		{
			input: model.MAgentParam{
				Time: "60d", //no data
			},
			expected: 20313,
			err:      nil,
		},
		{
			input: model.MAgentParam{
				Time: "5m", //mocking error return
			},
			expected: 21000,
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetCPUData(test.input)
		output := result.Result.Status.Code
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}

}
