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
func TestAddSpareCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "addspare", "--spare", "device0", "--array-name", "Array0", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"ADDDEVICE","rid":"fromCLI","param":{"array":"Array0",` +
		`"spare":[{"deviceName":"device0"}]}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q\n", expected, string(out))
	}
}
