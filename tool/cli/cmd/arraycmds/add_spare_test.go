package arraycmds_test

import (
	"bytes"
	"cli/cmd"
	"cli/cmd/testmgr"
	"log"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestAddSpareCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "addspare", "--spare", "device0", "--array-name", "Array0", "--json-req")

	output := buff.String()
	output = output[:len(output)-1] // Remove the last n from output string
	expected := `{"command":"ADDDEVICE","rid":"fromCLI","param":{"array":"Array0","spare":[{"deviceName":"device0"}]}}`

	if expected != output {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}
