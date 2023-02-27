package transportcmds

import (
	"github.com/spf13/cobra"
)

// This is a pseudo command to contain transport subcommands.
// TransportCmd itself does nothing when called.
var TransportCmd = &cobra.Command{
	Use:   "transport",
	Short: "Transport Command for PoseidonOS.",
	Long: `
Transport Command for PoseidonOS. Use this command category when you
create or list a transport. 

Syntax: 
  poseidonos-cli transport [create|list] [flags]
	  `,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
	},
}

func init() {
	// Add subcommands to transport command.
	// If you create a new subcommand, add it here.
	TransportCmd.AddCommand(CreateTransportCmd)
	TransportCmd.AddCommand(ListTransportCmd)
}
