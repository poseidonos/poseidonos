package volumecmds

import (
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"
	"encoding/json"
	"os"
	"pnconnector/src/log"

	"github.com/spf13/cobra"
)

var MountVolumeCmd = &cobra.Command{
	Use:   "mount [flags]",
	Short: "Mount a volume to the host.",
	Long: `
Mount a volume to the host. 

Syntax:
	mount (--volume-name | -v) VolumeName (--array-name | -a) ArrayName
	[(--subnqn | -q) TargetNVMSubsystemNVMeQualifiedName] [(--trtype | -t) TransportType]
	[(--traddr | -i) TargetAddress] [(--trsvcid | -p) TransportServiceId]

Example: 
	poseidonos-cli volume mount --volume-name Volume0 --array-name Volume0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {
		var requestList []messages.Request
		command := "CREATESUBSYSTEMAUTO"

		CheckSubsystemParam(cmd)

		// Execute create subsystem or add listener command
		// if related flag is input.
		if mount_volume_ready_to_create_subsystem == true {
			var warningMsg = "WARNING: Are you sure you want to mount volume to the subsystem:" +
				" " + mount_volume_subNqnName + "?\n" +
				`If the specified subsystem does not exist, a new subsystem will be created,
				and this volume will be mounted to it.`

			if mount_volume_isForced == false {
				conf := displaymgr.AskConfirmation(warningMsg)
				if conf == false {
					os.Exit(0)
				}
			}

			param := messages.CreateSubsystemAutoParam{
				SUBNQN: mount_volume_subNqnName,
			}

			uuid := globals.GenerateUUID()

			createSubsystemReq := messages.BuildReqWithParam(command, uuid, param)
			requestList = append(requestList, createSubsystemReq)
		}

		if mount_volume_ready_to_add_Listener == true {
			addListenerParam := messages.AddListenerParam{
				SUBNQN:             mount_volume_subNqnName,
				TRANSPORTTYPE:      mount_volume_trtype,
				TARGETADDRESS:      mount_volume_traddr,
				TRANSPORTSERVICEID: mount_volume_trsvcid,
			}

			uuid := globals.GenerateUUID()

			addListenerReq := messages.Request{
				RID:     uuid,
				COMMAND: "ADDLISTENER",
				PARAM:   addListenerParam,
			}
			requestList = append(requestList, addListenerReq)
		}

		mountVolumeParam := messages.MountVolumeParam{
			VOLUMENAME: mount_volume_volumeName,
			SUBNQN:     mount_volume_subNqnName,
			ARRAYNAME:  mount_volume_arrayName,
		}

		uuid := globals.GenerateUUID()

		mountVolumeReq := messages.Request{
			RID:     uuid,
			COMMAND: "MOUNTVOLUME",
			PARAM:   mountVolumeParam,
		}
		requestList = append(requestList, mountVolumeReq)

		// Do not send request to server and print response when testing request build.
		if !(globals.IsTestingReqBld) {
			for _, request := range requestList {
				reqJSON, err := json.Marshal(request)
				if err != nil {
					log.Error("error:", err)
				}

				displaymgr.PrintRequest(string(reqJSON))

				resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))

				displaymgr.PrintResponse(request.COMMAND, resJSON, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)
			}
		}
	},
}

func CheckSubsystemParam(cmd *cobra.Command) {
	if cmd.Flags().Changed("subnqn") {
		mount_volume_ready_to_create_subsystem = true
		if cmd.Flags().Changed("transport-type") &&
			cmd.Flags().Changed("target-address") &&
			cmd.Flags().Changed("transport-service-id") {
			mount_volume_ready_to_add_Listener = true
		}
	}
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_volume_volumeName = ""
var mount_volume_arrayName = ""
var mount_volume_subNqnName = ""
var mount_volume_trtype = ""
var mount_volume_traddr = ""
var mount_volume_trsvcid = ""
var mount_volume_ready_to_create_subsystem = false
var mount_volume_ready_to_add_Listener = false
var mount_volume_isForced = false

func init() {
	MountVolumeCmd.Flags().StringVarP(&mount_volume_volumeName,
		"volume-name", "v", "",
		"The name of the volume to mount.")
	MountVolumeCmd.MarkFlagRequired("volume-name")

	MountVolumeCmd.Flags().StringVarP(&mount_volume_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume belongs to.")
	MountVolumeCmd.MarkFlagRequired("array-name")

	MountVolumeCmd.Flags().StringVarP(&mount_volume_subNqnName,
		"subnqn", "q", "",
		`NVMe qualified name of target NVM subsystem. When this flag is specified,
		POS will check if the specified NVM subsystem exists. If it exists, 
		POS will mount this volume to it. Otherwise, POS will create a new
		NVM subsystem and mount this volume to it.`)

	MountVolumeCmd.Flags().StringVarP(&mount_volume_trtype,
		"transport-type", "t", "",
		"NVMe-oF transport type (ex. tcp)")

	MountVolumeCmd.Flags().StringVarP(&mount_volume_traddr,
		"target-address", "i", "",
		"NVMe-oF target address (ex. 127.0.0.1)")

	MountVolumeCmd.Flags().StringVarP(&mount_volume_trsvcid,
		"transport-service-id", "p", "",
		"NVMe-oF transport service id (ex. 1158)")

	MountVolumeCmd.Flags().BoolVarP(&mount_volume_isForced,
		"force", "", false,
		"Force to mount this volume.")
}
