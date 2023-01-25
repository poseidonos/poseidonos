// Package pos contains the code for executing commands on POS
package pos

import (
	pb "kouros/api"
)

type POSInterface int64

const (
	GRPC POSInterface = iota
)

// POManager defines the methods that any POS Manager should be implementing
type POSManager interface {

	// Init method is for initializing the Manager
	// The function takes 2 parameters
	// 1. client specifies the name of the client
	// 2. connectionInfo specifies the parameters for connection to POS
	Init(client string, connectionInfo interface{}) error

	// StartPoseidonOS starts the PoseidonOS
	// The function takes a command in JSON format as the parameter
	// A successful Create call returns response in JSON format
	StartPoseidonOS() (response []byte, err error)

	// StopPoseidonos method the running PoseidonOS
	// The function does not require any parameters
	// A successful Create call returns response in the Protobuf format of StopSystemResponse
	StopPoseidonOS() (response *pb.StopSystemResponse, request *pb.StopSystemRequest, err error)

	// GetSystemInfo method the running PoseidonOS
	// The function does not require any parameters
	// A successful Create call returns response in the Protobuf format of SystemInfoResponse
	GetSystemInfo() (response *pb.SystemInfoResponse, request *pb.SystemInfoRequest, err error)

	// GetSystemProperty method the running PoseidonOS
	// The function does not require any parameters
	// A successful Create call returns response in the Protobuf format of GetSystemPropertyResponse
	GetSystemProperty() (response *pb.GetSystemPropertyResponse, request *pb.GetSystemPropertyRequest, err error)

	// SetSystemProperty method creates an array in PoseidonOS
	// The function takes a command in Protobuf format of SetSystemPropertyRequest_Param as the parameter
	// A successful Create call returns response in the Protobuf format of SetSystemPropertyResponse
	SetSystemProperty(param *pb.SetSystemPropertyRequest_Param) (response *pb.SetSystemPropertyResponse, request *pb.SetSystemPropertyRequest, err error)

	// CreateDevice method creates an array in PoseidonOS
	// The function takes a command in Protobuf format of CreateDeviceRequest_Param as the parameter
	// A successful Create call returns response in the Protobuf format of CreateDeviceResponse
	CreateDevice(param *pb.CreateDeviceRequest_Param) (response *pb.CreateDeviceResponse, request *pb.CreateDeviceRequest, err error)

	// ScanDevice method creates an array in PoseidonOS
	// The function takes a command in Protobuf format of ScanDeviceRequest_Param as the parameter
	// A successful Create call returns response in the Protobuf format of ScanDeviceResponse
	ScanDevice() (reponse *pb.ScanDeviceResponse, request *pb.ScanDeviceRequest, err error)

	// GetDeviceSmartLog method creates an array in PoseidonOS
	// The function takes a command in Protobuf format of GetSmartLogRequest_Param as the parameter
	// A successful Create call returns response in the Protobuf format of GetSmartLogResponse
	GetDeviceSmartLog(param *pb.GetSmartLogRequest_Param) (response *pb.GetSmartLogResponse, request *pb.GetSmartLogRequest, err error)

	// ListDevices method lists the devices in PoseidonOS
	// The function does not require any parameters
	// A successful ListDevices call returns response in protobuf format
	ListDevices() (*pb.ListDeviceResponse, *pb.ListDeviceRequest, error)

	// Create Array method creates an array in PoseidonOS
	// The function takes a command in protobuf format as the parameter
	// A successful Create call returns response in protobuf format
	CreateArray(param *pb.CreateArrayRequest_Param) (*pb.CreateArrayResponse, *pb.CreateArrayRequest, error)

	// Add device command add spare device in PoseidonOS Array
	// The function takes a protobuf format as parameter and returns response in protobuf format
	AddSpareDevice(param *pb.AddSpareRequest_Param) (*pb.AddSpareResponse, *pb.AddSpareRequest, error)

	// Remove device command removes spare device from PoseidonOS Array
	// The function takes a protobuf format as parameter and returns response in protobuf format
	RemoveSpareDevice(param *pb.RemoveSpareRequest_Param) (*pb.RemoveSpareResponse, *pb.RemoveSpareRequest, error)

	// Automatically create an array for PoseidonOS with the number of devices the user specifies.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	AutoCreateArray(param *pb.AutocreateArrayRequest_Param) (*pb.AutocreateArrayResponse, *pb.AutocreateArrayRequest, error)

	// Delete an array from PoseidonOS. After executing this command, the data and volumes in the array will be deleted too.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	DeleteArray(param *pb.DeleteArrayRequest_Param) (*pb.DeleteArrayResponse, *pb.DeleteArrayRequest, error)

	// ArrayInfo command will display detailed information of an array.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ArrayInfo(param *pb.ArrayInfoRequest_Param) (*pb.ArrayInfoResponse, *pb.ArrayInfoRequest, error)

	// ListArray command will display detailed information of all existing arrays.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ListArray() (*pb.ListArrayResponse, *pb.ListArrayRequest, error)

	// MountArray command will mount the array, You can create a volume from an array only when the array is mounted.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	MountArray(param *pb.MountArrayRequest_Param) (*pb.MountArrayResponse, *pb.MountArrayRequest, error)

	// UnmountArray command will unmount the array, all the volumes in the array will be unmounted
	// The function takes a protobuf format as parameter and returns response in protobuf format
	UnmountArray(param *pb.UnmountArrayRequest_Param) (*pb.UnmountArrayResponse, *pb.UnmountArrayRequest, error)

	// Replace a data device with an available spare device in array. Use this command when you expect a possible problem of a data device. If there is no available spare device, this command will fail.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ReplaceArrayDevice(param *pb.ReplaceArrayDeviceRequest_Param) (*pb.ReplaceArrayDeviceResponse, *pb.ReplaceArrayDeviceRequest, error)

	// Reset the weights for backend events such as Flush, Rebuild, and GC to the default values.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ResetEventWRRPolicy() (*pb.ResetEventWrrResponse, *pb.ResetEventWrrRequest, error)

	// Reset MBR information of PoseidonOS. Use this command when you need to remove the all the arrays and reset the states of the devices.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ResetMBR() (*pb.ResetMbrResponse, *pb.ResetMbrRequest, error)

	StopRebuilding(param *pb.StopRebuildingRequest_Param) (*pb.StopRebuildingResponse, *pb.StopRebuildingRequest, error)

	UpdateEventWRRPolicy(param *pb.UpdateEventWrrRequest_Param) (*pb.UpdateEventWrrResponse, *pb.UpdateEventWrrRequest, error)

	DumpMemorySnapshot(param *pb.DumpMemorySnapshotRequest_Param) (*pb.DumpMemorySnapshotResponse, *pb.DumpMemorySnapshotRequest, error)

	// Apply a filtering policy to logger.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	ApplyLogFilter() (*pb.ApplyLogFilterResponse, *pb.ApplyLogFilterRequest, error)

	GetLogLevel() (*pb.GetLogLevelResponse, *pb.GetLogLevelRequest, error)

	// Display the current preference of logger
	// The function takes a protobuf format as parameter and returns response in protobuf format
	LoggerInfo() (*pb.LoggerInfoResponse, *pb.LoggerInfoRequest, error)

	SetLogLevel(param *pb.SetLogLevelRequest_Param) (*pb.SetLogLevelResponse, *pb.SetLogLevelRequest, error)

	// Set the preferences (e.g., format) of logger.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	SetLogPreference(param *pb.SetLogPreferenceRequest_Param) (*pb.SetLogPreferenceResponse, *pb.SetLogPreferenceRequest, error)

	// Add a listener to an NVMe-oF subsystem
	// The function takes a protobuf format as parameter and returns response in protobuf format
	AddListener(param *pb.AddListenerRequest_Param) (*pb.AddListenerResponse, *pb.AddListenerRequest, error)

	// Create an NVMe-oF subsystem to PoseidonOS.
	// The function takes a protobuf format as parameter and returns response in protobuf format
	CreateSubsystem(param *pb.CreateSubsystemRequest_Param) (*pb.CreateSubsystemResponse, *pb.CreateSubsystemRequest, error)

	CreateSubsystemAuto(param *pb.CreateSubsystemRequest_Param) (*pb.CreateSubsystemResponse, *pb.CreateSubsystemRequest, error)

	// Create NVMf transport to PoseidonOS
	// The function takes a protobuf format as parameter and returns response in protobuf format
	CreateTransport(param *pb.CreateTransportRequest_Param) (*pb.CreateTransportResponse, *pb.CreateTransportRequest, error)

	// Delete a subsystem from PoseidonOS
	// The function takes a protobuf format as parameter and returns response in protobuf format
	DeleteSubsystem(param *pb.DeleteSubsystemRequest_Param) (*pb.DeleteSubsystemResponse, *pb.DeleteSubsystemRequest, error)

	SubsystemInfo(param *pb.SubsystemInfoRequest_Param) (*pb.SubsystemInfoResponse, *pb.SubsystemInfoRequest, error)

	ListSubsystem() (*pb.ListSubsystemResponse, *pb.ListSubsystemRequest, error)

	// Start the collection of telemetry data in PoseidonOS
	// The function takes a protobuf format as parameter and returns response in protobuf format
	StartTelemetry() (*pb.StartTelemetryResponse, *pb.StartTelemetryRequest, error)

	// Stop the collection of telemetry data in PoseidonOS
	// The function takes a protobuf format as parameter and returns response in protobuf format
	StopTelemetry() (*pb.StopTelemetryResponse, *pb.StopTelemetryRequest, error)

	// Create the volume in PoseidonOS Array
	// The function takes a protobuf format as parameter and returns response in protobuf format
	CreateVolume(param *pb.CreateVolumeRequest_Param) (*pb.CreateVolumeResponse, *pb.CreateVolumeRequest, error)

	// Set property in telemetry
	// The function takes a protobuf format as parameter and returns response in protobuf format
	SetTelemetryProperty(param *pb.SetTelemetryPropertyRequest_Param) (*pb.SetTelemetryPropertyResponse, *pb.SetTelemetryPropertyRequest, error)

	// Get the property of telemetry configuration
	// The function returns response in protobuf format
	GetTelemetryProperty() (*pb.GetTelemetryPropertyResponse, *pb.GetTelemetryPropertyRequest, error)

	RebuildArray(param *pb.RebuildArrayRequest_Param) (*pb.RebuildArrayResponse, *pb.RebuildArrayRequest, error)

	ListVolume(param *pb.ListVolumeRequest_Param) (*pb.ListVolumeResponse, *pb.ListVolumeRequest, error)

	VolumeInfo(param *pb.VolumeInfoRequest_Param) (*pb.VolumeInfoResponse, *pb.VolumeInfoRequest, error)

	VolumeRename(param *pb.VolumeRenameRequest_Param) (*pb.VolumeRenameResponse, *pb.VolumeRenameRequest, error)

	VolumeProperty(param *pb.SetVolumePropertyRequest_Param) (*pb.SetVolumePropertyResponse, *pb.SetVolumePropertyRequest, error)

	DeleteVolume(param *pb.DeleteVolumeRequest_Param) (*pb.DeleteVolumeResponse, *pb.DeleteVolumeRequest, error)

	MountVolume(param *pb.MountVolumeRequest_Param) (*pb.MountVolumeResponse, *pb.MountVolumeRequest, error)

	UnmountVolume(param *pb.UnmountVolumeRequest_Param) (*pb.UnmountVolumeResponse, *pb.UnmountVolumeRequest, error)

	CreateQoSVolumePolicy(param *pb.QosCreateVolumePolicyRequest_Param) (*pb.QosCreateVolumePolicyResponse, *pb.QosCreateVolumePolicyRequest, error)

	ResetQoSVolumePolicy(param *pb.QosResetVolumePolicyRequest_Param) (*pb.QosResetVolumePolicyResponse, *pb.QosResetVolumePolicyRequest, error)

	ListQoSVolumePolicy(param *pb.ListQOSPolicyRequest_Param) (*pb.ListQOSPolicyResponse, *pb.ListQOSPolicyRequest, error)

    ListWBT() (*pb.ListWBTResponse, *pb.ListWBTRequest, error)

    WBT(param *pb.WBTRequest_Param) (*pb.WBTResponse, *pb.WBTRequest, error)

    WithTimeout(timeout uint32) (POSManager)
}
