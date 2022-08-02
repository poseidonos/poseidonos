package systemcmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestGetSystemProperty(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "system", "get-property")
	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}
