package volumecmds_test

import (
	"cli/cmd"
	"cli/cmd/globals"
	"cli/cmd/testmgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestCreateVolumeCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "volume", "create", "--volume-name", "vol01", "--array-name", "Array0", "--size", "4194304gb", "--maxiops", "5000", "--maxbw", "6000", "--json-req", "--force")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"CREATEVOLUME","rid":"fromfakeclient",` +
		`"param":{"name":"vol01","size":4194304,"maxiops":5000,"maxbw":6000,"array":"Array0"}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateVolumeCommandReqWithKB(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "volume", "create", "--volume-name", "vol01", "--array-name", "Array0", "--size", "5484000KB", "--maxiops", "5000", "--maxbw", "6000", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"CREATEVOLUME","rid":"fromfakeclient",` +
		`"param":{"name":"vol01","size":5615616000,"maxiops":5000,"maxbw":6000,"array":"Array0"}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateVolumeCommandReqWithGB(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "volume", "create", "--volume-name", "vol01", "--array-name", "Array0", "--size", "200GiB", "--maxiops", "5000", "--maxbw", "6000", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"CREATEVOLUME","rid":"fromfakeclient",` +
		`"param":{"name":"vol01","size":214748364800,"maxiops":5000,"maxbw":6000,"array":"Array0"}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateVolumeCommandReqWithTB(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "volume", "create", "--volume-name", "vol01", "--array-name", "Array0", "--size", "500TB", "--maxiops", "5000", "--maxbw", "6000", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"CREATEVOLUME","rid":"fromfakeclient",` +
		`"param":{"name":"vol01","size":549755813888000,"maxiops":5000,"maxbw":6000,"array":"Array0"}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}

func TestCreateVolumeCommandReqWhenVolNameHasASpecialCharacter(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "volume", "create", "--volume-name", "vol01%", "--array-name", "Array0", "--size", "500TB", "--maxiops", "5000", "--maxbw", "6000", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"Request": {"command":"CREATEVOLUME","rid":"83179da1-c763-11ec-8c2a-b42e99ff989b","param":{"name":"vol01%","array":"Array0","size":549755813888000,"maxiops":5000,"maxbw":6000},"requestor":"cli"} }
`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}
