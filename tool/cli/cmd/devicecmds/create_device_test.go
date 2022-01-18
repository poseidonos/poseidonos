package devicecmds_test

import (
	"cli/cmd"
	"cli/cmd/globals"
	"cli/cmd/testmgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestCreateDeviceCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "device", "create", "--device-name", "dev0", "--num-blocks", "512", "--block-size", "4096", "--device-type", "uram", "--numa", "0", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"CREATEDEVICE","rid":"fromfakeclient","param":{"name":"dev0","num_blocks":512,"block_size":4096,"dev_type":"uram","numa":0}} }
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateDeviceCommandReqWithDefaultArgs(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "device", "create", "--device-name", "dev0", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"CREATEDEVICE","rid":"fromfakeclient","param":{"name":"dev0","num_blocks":512,"block_size":8388608,"dev_type":"uram","numa":0}} }
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %s", expected, string(out))
	}
}
