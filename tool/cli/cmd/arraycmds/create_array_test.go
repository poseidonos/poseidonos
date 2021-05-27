package arraycmds_test

import (
	"bytes"
	"cli/cmd"
	"cli/cmd/testmgr"
	"log"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestCreateArrayCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	var buff bytes.Buffer
	log.SetOutput(&buff)
	log.SetFlags(0)

	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "device0", "--data-devs",
		"device1,device2,device3", "--spare", "device4", "--array-name", "Array0", "--raid", "RAID5", "--json-req")

	output := buff.String()
	output = output[:len(output)-1] // Remove the last n from output string
	expected := `{"command":"CREATEARRAY","rid":"fromfakeclient","param":{"name":"Array0","raidtype":"RAID5","buffer":[{"deviceName":"device0"}],"data":[{"deviceName":"device1"},{"deviceName":"device2"},{"deviceName":"device3"}]}}`

	if expected != output {
		t.Errorf("Expected: %q Output: %q", expected, output)
	}
}
