package displaymgr_test

import (
	"bytes"
	"cli/cmd/displaymgr"
	"cli/cmd/testmgr"
	"fmt"
	"log"
	"testing"
)

// This testing tests if the response is parsed and displayed well in human readable form for LISTARRAY command
func TestListArrayResHumanReadable(t *testing.T) {
	var command = "LISTARRAY"
	var resJSON = `{"command":"LISTARRAY","rid":"fromCLI",
	"result":{"status":{"code":0,"description":"DONE"},
	"data":{"arrayList": [{"createDatetime": "2021-04-16 15:52:14 +0900",
	"devicelist": [{"name": "uram0","type": "BUFFER"},
	{"name": "S4H2NE0M600736 ","type": "DATA"},
	{"name": "S4H2NE0M600745","type": "DATA"},
	{"name": "S4H2NE0M600763 ","type": "DATA"}],
	"name": "ARRAY0","status":"Mounted","updateDatetime": "2021-04-16 15:52:14 +0900"},
	{"createDatetime": "2021-04-16 15:52:14 +0900","devicelist": [{"name": "uram1","type": "BUFFER"},
	{"name": "S4H2NE0M600744","type": "DATA"},{"name": "S4H2NE0M600743 ","type": "DATA"},
	{"name": "S4H2NE0M600746 ","type": "DATA"}],"name": "ARRAY1","status":"Unmounted",
	"updateDatetime": "2021-04-16 15:52:14 +0900"}]}}}`

	expected := `Name: ARRAY0
	---------------------------
	Datetime Created: 2021-04-16 15:52:14 +0900
	Datetime Updated: 2021-04-16 15:52:14 +0900
	Status: Mounted
	
	Devices
	-------------
	Name: uram0
	Type: BUFFER
	
	Name: S4H2NE0M600736 
	Type: DATA
	
	Name: S4H2NE0M600745
	Type: DATA
	
	Name: S4H2NE0M600763 
	Type: DATA
	
	
	Name: ARRAY1
	---------------------------
	Datetime Created: 2021-04-16 15:52:14 +0900
	Datetime Updated: 2021-04-16 15:52:14 +0900
	Status: Unmounted
	
	Devices
	-------------
	Name: uram1
	Type: BUFFER
	
	Name: S4H2NE0M600744
	Type: DATA
	
	Name: S4H2NE0M600743 
	Type: DATA
	
	Name: S4H2NE0M600746 
	Type: DATA`

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	displaymgr.PrintResponse(command, resJSON, false, false)

	output := buff.String()
	output = output[:len(output)-1] // Remove the last \n from output string

	dist := testmgr.Levenshtein([]rune(expected), []rune(output))

	fmt.Println(dist)
	if dist > 50 {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}
