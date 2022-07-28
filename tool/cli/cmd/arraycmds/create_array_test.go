package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/globals"
	"cli/cmd/testmgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestCreateArrayCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// Execute the command to test with argument
	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--spare", "devspare", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestCreateArrayCommandReqWithoutSpare(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// Execute the command to test with argument
	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestCreateArrayCommandReqWithSpareBufferList(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"CREATEARRAY","rid":"fromfakeclient",` +
		`"param":{"name":"Array0","buffer":[{"deviceName":"uram0"}],` +
		`"data":[{"deviceName":"device1"},{"deviceName":"device2"},` +
		`{"deviceName":"device3"}],"spare":[{"deviceName":"spare1"},` +
		`{"deviceName":"spare2"},{"deviceName":"spare3"},` +
		`{"deviceName":"spare4"}],"raidtype":"RAID5"}} }
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateArrayCommandReqWithNoBufferFlag(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--no-buffer", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"CREATEARRAY","rid":"7bf114be-7753-11ec-be08-005056adcaa2","param":{"name":"Array0","data":[{"deviceName":"device1"},{"deviceName":"device2"},{"deviceName":"device3"}],"spare":[{"deviceName":"spare1"},{"deviceName":"spare2"},{"deviceName":"spare3"},{"deviceName":"spare4"}],"raidtype":"RAID5"},"requestor":"cli"} }`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateArrayCommandReqWithNoBufferError(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"time":"2022-01-17T05:07:50.838819912Z","level":"ERROR","prefix":"-","file":"create_array.go","line":"41","message":"no buffer is specified."}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateArrayWithRAID10AndOddDataDevsCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--spare", "devspare", "--array-name", "Array0", "--raid", "RAID10", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `error: RAID10 only supports even number of data devices.
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}
