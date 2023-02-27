package subsystemcmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/socketmgr"
	pb "kouros/api"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var SetListenerAnaStateCmd = &cobra.Command{
	Use:   "set-listener-ana-state [flags]",
	Short: "Set a listener's ana state to an NVMe-oF subsystem",
	Long: `
Set a listener's ana state to an NVMe-oF subsystem.

Syntax:
	poseidonos-cli subsystem set-listener-ana-state (--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId (--anastate | -a) AnaState

Example:
	poseidonos-cli subsystem set-listener-ana-state -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.100.2.14 -p 1158 -a inaccessible

    `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "SETLISTENERANASTATE"
		uuid := globals.GenerateUUID()
		param := &pb.SetListenerAnaStateRequest_Param{
			Subnqn:             subsystem_setlisteneranastate_subnqn,
			TransportType:      subsystem_setlisteneranastate_trtype,
			TargetAddress:      subsystem_setlisteneranastate_traddr,
			TransportServiceId: subsystem_setlisteneranastate_trsvcid,
			AnaState:			subsystem_setlisteneranastate_anastate,
		}

		req := &pb.SetListenerAnaStateRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

		reqJson, err := protojson.MarshalOptions{
			EmitUnpopulated: true,
		}.Marshal(req)
		if err != nil {
			log.Fatalf("failed to marshal the protobuf request: %v", err)
		}

		if !(globals.IsTestingReqBld) {
			var resJson string

			if globals.EnableGrpc == false {
				resJson = socketmgr.SendReqAndReceiveRes(string(reqJson))
			} else {
				posMgr, err := grpcmgr.GetPOSManager()
				if err != nil {
					log.Fatalf("failed to connect to POS: %v", err)
				}
				res, req, err := posMgr.SetListenerAnaState(param)
				if err != nil {
					globals.PrintErrMsg(err)
					return
				}
				resByte, err := protojson.Marshal(res)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf response: %v", err)
				}
				resJson = string(resByte)
				reqJson, err = protojson.MarshalOptions{
					EmitUnpopulated: true,
				}.Marshal(req)
				if err != nil {
					log.Fatalf("failed to marshal the protobuf request: %v", err)
				}
			}
			displaymgr.PrintRequest(string(reqJson))
			displaymgr.PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
		}

	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var subsystem_setlisteneranastate_subnqn = ""
var subsystem_setlisteneranastate_trtype = ""
var subsystem_setlisteneranastate_traddr = ""
var subsystem_setlisteneranastate_trsvcid = ""
var subsystem_setlisteneranastate_anastate = ""

func init() {
	SetListenerAnaStateCmd.Flags().StringVarP(&subsystem_setlisteneranastate_subnqn,
		"subnqn", "q", "",
		"The NQN of the subsystem to set listener's ana state.")
		SetListenerAnaStateCmd.MarkFlagRequired("subnqn")

	SetListenerAnaStateCmd.Flags().StringVarP(&subsystem_setlisteneranastate_trtype,
		"trtype", "t", "",
		"NVMe-oF transport type: e.g., tcp")
	SetListenerAnaStateCmd.MarkFlagRequired("trtype")

	SetListenerAnaStateCmd.Flags().StringVarP(&subsystem_setlisteneranastate_traddr,
		"traddr", "i", "",
		"NVMe-oF target address: e.g., an ip address")
	SetListenerAnaStateCmd.MarkFlagRequired("traddr")

	SetListenerAnaStateCmd.Flags().StringVarP(&subsystem_setlisteneranastate_trsvcid,
		"trsvcid", "p", "",
		"NVMe-oF transport service id: e.g., a port number")
	SetListenerAnaStateCmd.MarkFlagRequired("trsvcid")

	SetListenerAnaStateCmd.Flags().StringVarP(&subsystem_setlisteneranastate_anastate,
		"anastate", "a", "",
		"NVMe-oF subsytem-listener's ana state: e.g., an ANA state")
	SetListenerAnaStateCmd.MarkFlagRequired("anastate")

}
