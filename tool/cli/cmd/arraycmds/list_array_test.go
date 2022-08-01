package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestListArrayCommandReqWithoutArrayName(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "list", "--json-req")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestListArrayCommandReqWithArrayName(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "list", "--array-name", "Array0")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}
