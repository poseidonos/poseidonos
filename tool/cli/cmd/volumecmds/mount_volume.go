package volumecmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"fmt"
	"os"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
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
	RunE: func(cmd *cobra.Command, args []string) error {

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

			sendCreateSubsystemAuto()
		}

		if mount_volume_ready_to_add_Listener == true {
			sendAddListener()
		}

		sendMountVolume()

		return nil
	},
}

func sendCreateSubsystemAuto() error {

	command := "CREATESUBSYSTEMAUTO"
	req, buildErr := buildCreateSubsystemAutoReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendCreateSubsystem(req)
	if gRpcErr != nil {
		globals.PrintErrMsg(gRpcErr)
		return gRpcErr
	}

	printErr := displaymgr.PrintProtoResponse(command, res)
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

func sendAddListener() error {
	command := "ADDLISTENER"
	req, buildErr := buildAddListenerReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendAddListener(req)
	if gRpcErr != nil {
		globals.PrintErrMsg(gRpcErr)
		return gRpcErr
	}

	printErr := displaymgr.PrintProtoResponse(command, res)
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

func sendMountVolume() error {
	command := "MOUNTVOLUME"

	req, buildErr := buildMountVolumeReq(command)
	if buildErr != nil {
		fmt.Printf("failed to build request: %v", buildErr)
		return buildErr
	}

	reqJson, err := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))

	res, gRpcErr := grpcmgr.SendMountVolume(req)
	if gRpcErr != nil {
		globals.PrintErrMsg(gRpcErr)
		return gRpcErr
	}

	printErr := displaymgr.PrintProtoResponse(command, res)
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

func buildMountVolumeReq(command string) (*pb.MountVolumeRequest, error) {
	param := &pb.MountVolumeRequest_Param{
		Name:   mount_volume_volumeName,
		Subnqn: mount_volume_subNqnName,
		Array:  mount_volume_arrayName,
	}
	uuid := globals.GenerateUUID()
	req := &pb.MountVolumeRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

func buildCreateSubsystemAutoReq(command string) (*pb.CreateSubsystemRequest, error) {
	param := &pb.CreateSubsystemRequest_Param{
		Nqn: mount_volume_subNqnName,
	}
	uuid := globals.GenerateUUID()
	req := &pb.CreateSubsystemRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

func buildAddListenerReq(command string) (*pb.AddListenerRequest, error) {
	param := &pb.AddListenerRequest_Param{
		Subnqn:             mount_volume_subNqnName,
		TransportType:      mount_volume_trtype,
		TargetAddress:      mount_volume_traddr,
		TransportServiceId: mount_volume_trsvcid,
	}
	uuid := globals.GenerateUUID()
	req := &pb.AddListenerRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
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
