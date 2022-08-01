package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

func TestCreateArray(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--spare", "devspare", "--array-name", "Array0", "--raid", "RAID5")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestCreateArrayWithoutSpare(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--raid", "RAID5")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestCreateArrayWithSpareBufferList(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestCreateArrayWithNoBufferFlag(t *testing.T) {

	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--no-buffer", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5")

	expected := `unknown flag: --no-buffer`

	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}

func TestCreateArrayWithNoBufferError(t *testing.T) {

	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--data-devs",
		"device1,device2,device3", "--array-name", "Array0", "--spare", "spare1,spare2,spare3,spare4", "--raid", "RAID5")

	expected := `required flag(s) "buffer" not set`

	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}

func TestCreateArrayRAID10WithOddDataDevs(t *testing.T) {

	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "create", "--buffer", "uram0", "--data-devs",
		"device1,device2,device3", "--spare", "devspare", "--array-name", "Array0", "--raid", "RAID10")

	expected := `RAID10 only supports even number of data devices.`

	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}
