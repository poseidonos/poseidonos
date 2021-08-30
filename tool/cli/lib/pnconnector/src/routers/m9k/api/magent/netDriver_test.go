package magent

import (
	"pnconnector/src/routers/m9k/api/magent/mocks"
	"pnconnector/src/routers/m9k/model"
	"reflect"
	"testing"
)

func TestGetNetDriver(t *testing.T) {
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
			expected: NetDriverFields{
				{
					Interface: "interface",
					Driver:    "driver",
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
			dbName:   "poseidonDataNil",
			expected: []string{},
			err:      nil,
		},
	}

	IDBClient = mocks.MockInfluxClient{}
	for _, test := range tests {
		DBName = test.dbName
		result, err := GetNetDriver(test.input)
		output := result.Result.Data
		if !reflect.DeepEqual(output, test.expected) || err != test.err {
			t.Errorf("Test Failed: %v inputted, %v expected, received: %v, received err: %v", test.input, test.expected, output, err)
		}
	}
	DBName = actualDBName

}
