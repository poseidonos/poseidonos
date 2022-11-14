package volumecmds

import (
	pb "cli/api"
	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/grpcmgr"
	"cli/cmd/messages"
	"fmt"

	"github.com/spf13/cobra"
	"google.golang.org/protobuf/encoding/protojson"
)

var MountVolumeWithSubsystemCmd = &cobra.Command{
	Use:   "mount-with-subsystem [flags]",
	Short: "Create a subsystem and add listener automatically. Mount a volume to Host.",
	Long: `
Create a subsystem and add listener automatically and then mount a volume to Host.

Syntax:
	mount-with-subsystem (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 
	(--subnqn | -q) SubsystemNQN (--trtype | -t) TransportType (--traddr | -i) TargetAddress (--trsvcid | -p) TransportServiceId

Example: 
	poseidonos-cli volume mount-with-subsystem --volume-name vol1 --subnqn nqn.2019-04.ibof:subsystem1 
	--array-name POSArray --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var requestList [3]messages.Request

		createSubsysCmd := "CREATESUBSYSTEMAUTO"
		createSubsysParam := messages.CreateSubsystemAutoParam{
			SUBNQN: mount_volume_with_subsystem_subnqn,
		}

		uuid := globals.GenerateUUID()

		createSubsystemReq := messages.BuildReqWithParam(createSubsysCmd, uuid, createSubsysParam)
		requestList[0] = createSubsystemReq

		sendCreateSubsystemAutoWithSub()
		sendAddListenerWithSub()
		sendMountVolumeWithSub()
	},
}

func sendCreateSubsystemAutoWithSub() error {

	command := "CREATESUBSYSTEMAUTO"
	req, buildErr := buildCreateSubsystemAutoReqWithSub(command)
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

func sendAddListenerWithSub() error {
	command := "ADDLISTENER"
	req, buildErr := buildAddListenerReqWithSub(command)
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

func sendMountVolumeWithSub() error {
	command := "MOUNTVOLUME"

	req, buildErr := buildMountVolumeReqWithSub(command)
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

func buildMountVolumeReqWithSub(command string) (*pb.MountVolumeRequest, error) {
	param := &pb.MountVolumeRequest_Param{
		Name:   mount_volume_with_subsystem_volumeName,
		Subnqn: mount_volume_with_subsystem_subnqn,
		Array:  mount_volume_with_subsystem_arrayName,
	}
	uuid := globals.GenerateUUID()
	req := &pb.MountVolumeRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

func buildCreateSubsystemAutoReqWithSub(command string) (*pb.CreateSubsystemRequest, error) {
	param := &pb.CreateSubsystemRequest_Param{
		Nqn: mount_volume_with_subsystem_subnqn,
	}
	uuid := globals.GenerateUUID()
	req := &pb.CreateSubsystemRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

func buildAddListenerReqWithSub(command string) (*pb.AddListenerRequest, error) {
	param := &pb.AddListenerRequest_Param{
		Subnqn:             mount_volume_with_subsystem_subnqn,
		TransportType:      mount_volume_with_subsystem_trtype,
		TargetAddress:      mount_volume_with_subsystem_traddr,
		TransportServiceId: mount_volume_with_subsystem_trsvcid,
	}
	uuid := globals.GenerateUUID()
	req := &pb.AddListenerRequest{Command: command, Rid: uuid, Requestor: "cli", Param: param}

	return req, nil
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_volume_with_subsystem_volumeName = ""
var mount_volume_with_subsystem_subnqn = ""
var mount_volume_with_subsystem_arrayName = ""
var mount_volume_with_subsystem_trtype = ""
var mount_volume_with_subsystem_traddr = ""
var mount_volume_with_subsystem_trsvcid = ""

func init() {
	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_volumeName,
		"volume-name", "v", "",
		"The name of the volume to mount.")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("volume-name")

	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_subnqn,
		"subnqn", "q", "",
		"NQN of the subsystem to create.")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("subnqn")

	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_arrayName,
		"array-name", "a", "",
		"The name of the array where the volume belongs to.")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("array-name")

	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_trtype,
		"transport_type", "t", "",
		"NVMe-oF transport type (ex. tcp)")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("transport-type")

	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_traddr,
		"target_address", "i", "",
		"NVMe-oF target address (ex. 127.0.0.1)")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("target_address")

	MountVolumeWithSubsystemCmd.Flags().StringVarP(&mount_volume_with_subsystem_trsvcid,
		"transport_service_id", "p", "",
		"NVMe-oF transport service id (ex. 1158)")
	MountVolumeWithSubsystemCmd.MarkFlagRequired("transport-service-id")
}
