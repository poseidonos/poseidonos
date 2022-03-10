package subsystemcmds

import (
	"encoding/json"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var CreateSubsystemCmd = &cobra.Command{
	Use:   "create [flags]",
	Short: "Create an NVMe-oF subsystem to PoseidonOS.",
	Long: `Create an NVMe-oF subsystem to PoseidonOS.

Syntax:
	poseidonos-cli subsystem create (--subnqn | -q) SubsystemNQN 
	[--serial-number SerialNumber] [--model-number ModelNumber] 
	[(--max-namespaces | -m) MaxNamespace] [(--allow-any-host | -o)] [(--ana-reporting | -r)]

Example:
	poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 
	--serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION -m 256 -o
    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "CREATESUBSYSTEM"

		param := messages.CreateSubsystemParam{
			SUBNQN:        subsystem_create_subnqn,
			SERIAL:        subsystem_create_serial,
			MODEL:         subsystem_create_model,
			MAXNAMESPACES: subsystem_create_maxnamespace,
			ALLOWANYHOST:  subsystem_create_allowanyhost,
			ANAREPORTING:  subsystem_create_anareporting,
		}

		uuid := globals.GenerateUUID()

		req := messages.Request{
			RID:     uuid,
			COMMAND: command,
			PARAM:   param,
		}

		reqJSON, err := json.Marshal(req)
		if err != nil {
			log.Error("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
			displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var subsystem_create_subnqn = ""
var subsystem_create_serial = ""
var subsystem_create_model = ""
var subsystem_create_maxnamespace = 0
var subsystem_create_allowanyhost = false
var subsystem_create_anareporting = false

func init() {
	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_subnqn, "subnqn", "q", "", "NQN of the subsystem to create")
	CreateSubsystemCmd.MarkFlagRequired("subnqn")

	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_serial, "serial-number", "", "", "Serial Number of the subsystem to create. Default : POS00000000000000")
	CreateSubsystemCmd.Flags().StringVarP(&subsystem_create_model, "model-number", "", "", "Model Number of the subsystem to create. Default : POS_VOLUME_EXTENTION")
	CreateSubsystemCmd.Flags().IntVarP(&subsystem_create_maxnamespace, "max-namespaces", "m", 0, "Maximum number of namespaces allowed. Default : 256")
	CreateSubsystemCmd.Flags().BoolVarP(&subsystem_create_allowanyhost, "allow-any-host", "o", false, "Allow any host to connect (don't enforce host NQN whitelist). Default : false")
	CreateSubsystemCmd.Flags().BoolVarP(&subsystem_create_anareporting, "ana-reporting", "r", false, "Enable ANA reporting feature. Default : false")
}
