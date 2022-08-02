package systemcmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestSetSystemProperty(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "system", "set-property", "--rebuild-impact", "lowest")
	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}
