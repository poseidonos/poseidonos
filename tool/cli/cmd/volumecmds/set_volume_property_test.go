package volumecmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestSetVolumePropertyReqWithBothPrimaryAndSecondary(t *testing.T) {

	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "volume", "set-property", "--volume-name", "vol1",
		"--array-name", "arr1", "--primary-volume", "--secondary-volume")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}
