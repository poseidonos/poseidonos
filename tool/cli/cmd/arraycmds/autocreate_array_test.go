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
func TestAutocreateArrayCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--buffer", "uram0", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"AUTOCREATEARRAY","rid":"fromfakeclient",` +
		`"param":{"name":"Array0","buffer":[{"deviceName":"uram0"}],` +
		`"data":3,"spare":1,"raidtype":"RAID5"}} }
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestAutocreateArrayCommandReqWithNoBufferFlag(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--no-buffer", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"AUTOCREATEARRAY","rid":"8d3970b0-7753-11ec-9670-005056adcaa2","param":{"name":"Array0","num_data":3,"num_spare":1,"raidtype":"RAID5"},"requestor":"cli"} }`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestAutocreateArrayCommandReqWithNoBufferError(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"time":"2022-01-17T05:06:51.161390504Z","level":"ERROR","prefix":"-","file":"autocreate_array.go","line":"39","message":"no buffer is specified."}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestAutocreateArrayCommandReqWithRAID10AndOddDataDevs(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--no-buffer", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID10", "--json-req")

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
