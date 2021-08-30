package util

import (
	"pnconnector/src/log"
	"fmt"
	"os/exec"
)

func ExecCmd(cmd string, background bool) error {
	execCmd := exec.Command("sudo", "bash", "-c", cmd)
	output, err := execCmd.CombinedOutput()

	if err != nil {
		return fmt.Errorf("exec Run: %v %s", err, output)
	}

	log.Info(string(output))

	return nil
}
