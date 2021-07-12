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

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--spare", "devspare", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"CREATEARRAY","rid":"fromfakeclient","param":` +
		`{"name":"Array0","raidtype":"RAID5","buffer":[{"deviceName":"uram0"}],` +
		`"data":[{"deviceName":"device1"},{"deviceName":"device2"},` +
		`{"deviceName":"device3"}],"spare":[{"deviceName":"devspare"}]}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}
