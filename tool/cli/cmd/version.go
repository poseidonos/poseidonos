package cmd

import (
	"fmt"
	"github.com/spf13/cobra"
)

func init() {
	rootCmd.AddCommand(versionCmd)
}

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print the version number of dagent CLI",
	Long:  `All software has versions. This is dagent CLI's`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("dagent CLI Version 0.1.")
	},
}
