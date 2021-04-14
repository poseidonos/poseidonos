package magent

import (
	"pnconnector/src/routers/m9k/api/magent/mocks"
	"pnconnector/src/routers/m9k/model"
	"reflect"
	"testing"
)

func TestGetNetAddress(t *testing.T) {
	actualDBName := DBName
	var tests = []struct {
		input    model.MAgentParam
		dbName   string
		expected interface{}
		err      error
	}{
		{
			input:  model.MAgentParam{},
			dbName: "poseidon",
			expected: NetAddsFields{
				{
					Interface: "interface",
					Address:   "address",
				},
			},
			err: nil,
		},
		{
			input:    model.MAgentParam{},
			dbName:   "poseidonQueryErr",
			expected: []string{},
			err:      nil,
		},
		{
			input:    model.MAgentParam{},
			dbName:   "poseidonNoData",
			expected: []string{},
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		DBName = test.dbName
		result, err := GetNetAddress(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
	DBName = actualDBName

}
