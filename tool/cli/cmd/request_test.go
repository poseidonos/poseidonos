package cmd

import (
	"fmt"
	"github.com/spf13/cobra"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
	"time"
)

func TestSend(t *testing.T) {

	path, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	cmd := "cp -rf ../script " + path + "/../"
	cpCmd := exec.Command("/bin/sh", "-c", cmd)
	err := cpCmd.Run()

	if err != nil {
		fmt.Println(err)
	}

	cmd = "cp -rf ../bin/* " + path
	cpCmd = exec.Command("/bin/sh", "-c", cmd)
	err = cpCmd.Run()

	if err != nil {
		fmt.Println(err)
	}

	test1 := [][]string{
		[]string{"run_ibofos"},
		[]string{"scan_dev"},
		[]string{"create_array"},
		[]string{"list_array_device"},
		[]string{"mount_ibofos"},
		[]string{"create_vol"},
		[]string{"mount_vol"},
		[]string{"list_vol"},
		[]string{"unmount_vol"},
		[]string{"delete_vol"},
		[]string{"unmount_ibofos"},
		[]string{"exit_ibofos"},
	}

	for _, test := range test1 {

		t.Run(test[0], func(t *testing.T) {

			var cmd cobra.Command
			cmd.PersistentFlags().BoolVar(&isQuiet, "quiet", false, "")
			cmd.PersistentFlags().BoolVar(&isDebug, "debug", true, "")

			if test[0] == "create_array" {

				cmd.PersistentFlags().StringSliceVar(&buffer, "buffer", []string{"uram0"}, "")
                                cmd.PersistentFlags().Lookup("buffer").Changed = true
				cmd.PersistentFlags().StringSliceVar(&data, "data", []string{"unvme-ns-0", "unvme-ns-1", "unvme-ns-2"}, "")
                                cmd.PersistentFlags().Lookup("data").Changed = true
				cmd.PersistentFlags().StringSliceVar(&spare, "spare", []string{"unvme-ns-3"}, "")
                                cmd.PersistentFlags().Lookup("spare").Changed = true
			} else if test[0] == "create_vol" {

                                cmd.PersistentFlags().StringVar(&name, "name", "vol01", "")
                                cmd.PersistentFlags().Lookup("name").Changed = true
				cmd.PersistentFlags().StringVar(&size, "size", "4194304", "")
                                cmd.PersistentFlags().Lookup("size").Changed = true
				
			} else if test[0] == "mount_vol" {

				cmd.PersistentFlags().StringVar(&name, "name", "vol01", "")
                                cmd.PersistentFlags().Lookup("name").Changed = true
			} else if test[0] == "unmount_vol" {

				cmd.PersistentFlags().StringVar(&name, "name", "vol01", "")
                                cmd.PersistentFlags().Lookup("name").Changed = true
			} else if test[0] == "delete_vol" {

				cmd.PersistentFlags().StringVar(&name, "name", "vol01", "")
                                cmd.PersistentFlags().Lookup("name").Changed = true
			}

			res, err := Send(&cmd, test)

			time.Sleep(5 * time.Second)

			if err != nil || (res.Result.Status.Code != 0 && res.Result.Status.Code != 1022) {
				t.Error("error")
			}
		})
	}
}
