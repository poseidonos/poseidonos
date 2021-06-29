package magent

import (
	"pnconnector/src/routers/m9k/api/magent/mocks"
	"pnconnector/src/routers/m9k/model"
	"encoding/json"
	"reflect"
	"testing"
)

func TestGetNetData(t *testing.T) {
	var tests = []struct {
		input    model.MAgentParam
		expected interface{}
		err      error
	}{
		{
			input: model.MAgentParam{},
			expected: NetFields{
				{
					Time:        json.Number("1589872050483860738"),
					BytesRecv:   json.Number("10"),
					BytesSent:   json.Number("20"),
					DropIn:      json.Number("30"),
					DropOut:     json.Number("40"),
					ErrIn:       json.Number("50"),
					ErrOut:      json.Number("60"),
					PacketsRecv: json.Number("70"),
					PacketsSent: json.Number("80"),
				},
			},
			err: nil,
		},
		{
			input: model.MAgentParam{
				Time: "15m",
			},
			expected: NetFields{
				{
					Time:        json.Number("1589872050483860738"),
					BytesRecv:   json.Number("10"),
					BytesSent:   json.Number("20"),
					DropIn:      json.Number("30"),
					DropOut:     json.Number("40"),
					ErrIn:       json.Number("50"),
					ErrOut:      json.Number("60"),
					PacketsRecv: json.Number("70"),
					PacketsSent: json.Number("80"),
				},
				{
					Time:        json.Number("1589872050483870738"),
					BytesRecv:   json.Number("50"),
					BytesSent:   json.Number("60"),
					DropIn:      json.Number("30"),
					DropOut:     json.Number("40"),
					ErrIn:       json.Number("90"),
					ErrOut:      json.Number("60"),
					PacketsRecv: json.Number("70"),
					PacketsSent: json.Number("80"),
				},
			},
			err: nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetNetData(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}

}

func TestGetNetDataError(t *testing.T) {
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
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		result, err := GetNetData(test.input)
		output := result.Result.Status.Code
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
}
