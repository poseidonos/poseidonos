package qoscmds_test

import (
	"cli/cmd"
	"cli/cmd/globals"
	"cli/cmd/testmgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestVolumePolicyCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "qos", "create", "--volume-name", "vol01", "--array-name", "Array0", "--maxiops", "3000", "--maxbw", "2000", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout
	expected := `{"command":"QOSCREATEVOLUMEPOLICY","rid":"fromCLI","param":{"vol":[{"volumeName":"vol01"}],"miniops":-1,"maxiops":3000,"minbw":-1,"maxbw":2000,"array":"Array0"}}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %q", expected, string(out))
	}
}
