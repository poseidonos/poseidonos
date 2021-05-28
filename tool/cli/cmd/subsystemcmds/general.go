package subsystemcmds

import (
	"github.com/spf13/cobra"
)

// This is a pseudo command to contain array subcommands.
// ArryCmd itself does nothing when called.
var SubsystemCmd = &cobra.Command{
	Use:   "subsystem",
	Short: "Subsystem Command for PoseidonOS.",
	Long: `Subsystem Command for PoseidonOS.

Syntax: 
  poseidonos-cli subsystem [delete] [flags]
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	// Add subcommands to array command.
	// If you create a new subcommand, add it here.
	SubsystemCmd.AddCommand(DeleteSubsystemCmd)
}
