package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestAddSpare(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "addspare", "--spare", "device0", "--array-name", "Array0")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestAddSpareCommandReqWihtoutSpare(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "addspare", "--array-name", "Array0")

	expected := `required flag(s) "spare" not set`
	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}
