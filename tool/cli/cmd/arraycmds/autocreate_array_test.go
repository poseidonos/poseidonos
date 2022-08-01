package arraycmds_test

import (
	"cli/cmd"
	"cli/cmd/testmgr"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestAutocreateArray(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--buffer", "uram0", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5")

	if err != nil {
		t.Errorf("Expected: nil Output: %q", err.Error())
	}
}

func TestAutocreateArrayWithNoBufferFlag(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--no-buffer", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5")

	expected := `unknown flag: --no-buffer`

	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}

func TestAutocreateArrayWithNoBufferError(t *testing.T) {
	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID5")

	expected := `required flag(s) "buffer" not set`

	if err != nil {
		if err.Error() != expected {
			t.Errorf("Expected: %q Output: %q", expected, err.Error())
		}
	}
}

func TestAutocreateArrayWithRAID10AndOddDataDevs(t *testing.T) {

	rootCmd := cmd.RootCmd

	_, err := testmgr.ExecuteCommand(rootCmd, "array", "autocreate", "--buffer", "uram0", "--num-data-devs",
		"3", "--num-spare", "1", "--array-name", "Array0", "--raid", "RAID10")

	expected := `RAID10 only supports even number of data devices.`

	if err.Error() != expected {
		t.Errorf("Expected: %q Output: %q", expected, err.Error())
	}
}
