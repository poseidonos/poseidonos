package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestDeleteArrayCommandReq(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "delete", "--array-name", "Array0", "--force")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}
