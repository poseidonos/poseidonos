package pos

import (
	"errors"
	pb "kouros/api"
	"kouros/pos/grpc"
	"kouros/utils"
	"os"
	"os/exec"
	"path/filepath"
)

const dialTimeout = 10
const reqTimeout = 360

type POSGRPCManager struct {
	connection grpc.POSGRPCConnection
	requestor  string
}

func (p *POSGRPCManager) Init(client string, address interface{}) error {
	if grpcAddress, ok := address.(string); !ok {
		return errors.New("Please provide an address of type string")
	} else {
		p.connection = grpc.POSGRPCConnection{
			Address: grpcAddress,
            ReqTimeout: reqTimeout,
            TimeoutChanged: false,
		}
		p.requestor = client
	}
	return nil
}

func (p *POSGRPCManager) GetSystemProperty() (response *pb.GetSystemPropertyResponse, request *pb.GetSystemPropertyRequest, err error) {
	command := "GETSYSTEMPROPERTY"
	req := &pb.GetSystemPropertyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendGetSystemProperty(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) SetSystemProperty(param *pb.SetSystemPropertyRequest_Param) (response *pb.SetSystemPropertyResponse, request *pb.SetSystemPropertyRequest, err error) {
	command := "REBUILDPERFIMPACT"
	req := &pb.SetSystemPropertyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSetSystemProperty(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) StartPoseidonOS() ([]byte, error) {
	startScriptPath, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	startScriptPath += "/../script/start_poseidonos.sh"
	err := exec.Command("/bin/sh", "-c", "sudo "+startScriptPath).Run()
	resJSON := ""
	uuid := utils.GenerateUUID()
	if err != nil {
		resJSON = `{"command":"STARTPOS","rid":"` + uuid + `"` + `,"result":{"status":{"code":11000,` +
			`"description":"PoseidonOS has failed to start with error code: 11000"}}}`
	} else {
		resJSON = `{"command":"STARTPOS","rid":"` + uuid + `","result":{"status":{"code":0,` +
			`"description":"Done! PoseidonOS has started!"}}}`
	}
	res := []byte(resJSON)
	return res, nil
}

func (p *POSGRPCManager) StopPoseidonOS() (response *pb.StopSystemResponse, request *pb.StopSystemRequest, err error) {
	command := "STOPSYSTEM"
	req := &pb.StopSystemRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendStopSystem(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) GetSystemInfo() (response *pb.SystemInfoResponse, request *pb.SystemInfoRequest, err error) {
	command := "SYSTEMINFO"
	req := &pb.SystemInfoRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendSystemInfo(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) CreateDevice(param *pb.CreateDeviceRequest_Param) (*pb.CreateDeviceResponse, *pb.CreateDeviceRequest, error) {
	command := "CREATEDEVICE"
	req := &pb.CreateDeviceRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateDevice(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ScanDevice() (*pb.ScanDeviceResponse, *pb.ScanDeviceRequest, error) {
	command := "SCANDEVICE"
	req := &pb.ScanDeviceRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendScanDevice(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) GetDeviceSmartLog(param *pb.GetSmartLogRequest_Param) (*pb.GetSmartLogResponse, *pb.GetSmartLogRequest, error) {
	command := "SMARTLOG"
	req := &pb.GetSmartLogRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendGetSmartLog(p.connection, req)
	return res, req, err
}

// ListDevices method lists the devices in PoseidonOS
// The function does not require any parameters
// A successful ListDevices call returns response in protobuf format with the following fields
// rid: Request ID of the function (string)
// command: PoseidonOS Command name (string)
// result: Result object which contains the below fields
//   1. status: Status object which contains the below fields
//     - code: Status code of response (int32)
//     - event_name: Event Name (string)
//     - description: Description about  (string)
//     - cause: Cause of the error occured, if any (string)
//     - solution: Solution of the problem occured, if any (string)
//   2. data: Data object contains the below fileds
//    - devicelist: A list of devices. Each device contains the following fields
//        1. name: Name of the device (string)
//        2. type: Type of the device (eg. "SSD", "URAM") (string)
//        3. address: PCIe address of the device (string)
//        4. class: Class of device eg. "Array", "System" (string)
//        5. modelNumber: Model number of the device (string)
//        6. numa: NUMA value associated with the device (string)
//        7. size: Size of the device in bytes (int64)
//        8. serialNumber: Serial number of the device (string)
// info: info object contains the following fields
//    1. version: PoseidonOS version (string)
func (p *POSGRPCManager) ListDevices() (*pb.ListDeviceResponse, *pb.ListDeviceRequest, error) {
	command := "LISTDEVICE"
	req := &pb.ListDeviceRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendListDevice(p.connection, req)
	return res, req, err
}

// Create Array method creates an array in PoseidonOS
// The function takes a byte array in protobuf format as parameter with the following fields
// param: param object contains the following fields
//   - name: Name of the array (string)
//   - raidtype: RAID type of the array
//   - data: A list of data devices. Each device contains the following fields
//      1. deviceName: name of the device (string)
//   - buffer: A list of buffer devices. Each device contains the following fields
//      1. deviceName: name of the device (string)
//   - spare: A list of spare devices. Each device contains the following fields
//      1. deviceName: name of the device (string)
// A successful Create call returns response in protobuf format with the following fields
// rid: Request ID of the function (string)
// command: PoseidonOS Command name (string)
// result: Result object which contains the below fields
//   1. status: Status object which contains the below fields
//     - code: Status code of response (int32)
//     - event_name: Event Name (string)
//     - description: Description about  (string)
//     - cause: Cause of the error occured, if any (string)
//     - solution: Solution of the problem occured, if any (string)
// info: info object contains the following fields
//    1. version: PoseidonOS version (string)
func (p *POSGRPCManager) CreateArray(param *pb.CreateArrayRequest_Param) (*pb.CreateArrayResponse, *pb.CreateArrayRequest, error) {
	command := "CREATEARRAY"
	req := &pb.CreateArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateArray(p.connection, req)
	return res, req, err
}

// Add device command add spare device in PoseidonOS Array
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) AddSpareDevice(param *pb.AddSpareRequest_Param) (*pb.AddSpareResponse, *pb.AddSpareRequest, error) {
	command := "ADDDEVICE"
	req := &pb.AddSpareRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendAddSpare(p.connection, req)
	return res, req, err
}

// Remove device command removes spare device from PoseidonOS Array
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) RemoveSpareDevice(param *pb.RemoveSpareRequest_Param) (*pb.RemoveSpareResponse, *pb.RemoveSpareRequest, error) {
	command := "REMOVEDEVICE"
	req := &pb.RemoveSpareRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendRemoveSpare(p.connection, req)
	return res, req, err
}

// Automatically create an array for PoseidonOS with the number of devices the user specifies.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) AutoCreateArray(param *pb.AutocreateArrayRequest_Param) (*pb.AutocreateArrayResponse, *pb.AutocreateArrayRequest, error) {
	command := "AUTOCREATEARRAY"
	req := &pb.AutocreateArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendAutocreateArray(p.connection, req)
	return res, req, err
}

// Delete an array from PoseidonOS. After executing this command, the data and volumes in the array will be deleted too.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) DeleteArray(param *pb.DeleteArrayRequest_Param) (*pb.DeleteArrayResponse, *pb.DeleteArrayRequest, error) {
	command := "DELETEARRAY"
	req := &pb.DeleteArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendDeleteArray(p.connection, req)
	return res, req, err
}

// ArrayInfo command will display detailed information of an array.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ArrayInfo(param *pb.ArrayInfoRequest_Param) (*pb.ArrayInfoResponse, *pb.ArrayInfoRequest, error) {
	command := "ARRAYINFO"
	req := &pb.ArrayInfoRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendArrayInfo(p.connection, req)
	return res, req, err
}

// ListArray command will display detailed information of all existing arrays.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ListArray() (*pb.ListArrayResponse, *pb.ListArrayRequest, error) {
	command := "LISTARRAY"
	req := &pb.ListArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendListArray(p.connection, req)
	return res, req, err
}

// MountArray command will mount the array, You can create a volume from an array only when the array is mounted.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) MountArray(param *pb.MountArrayRequest_Param) (*pb.MountArrayResponse, *pb.MountArrayRequest, error) {
	command := "MOUNTARRAY"
	req := &pb.MountArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendMountArray(p.connection, req)
	return res, req, err
}

// UnmountArray command will unmount the array, all the volumes in the array will be unmounted
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) UnmountArray(param *pb.UnmountArrayRequest_Param) (*pb.UnmountArrayResponse, *pb.UnmountArrayRequest, error) {
	command := "UNMOUNTARRAY"
	req := &pb.UnmountArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendUnmountArray(p.connection, req)
	return res, req, err
}

// Replace a data device with an available spare device in array. Use this command when you expect a possible problem of a data device. If there is no available spare device, this command will fail.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ReplaceArrayDevice(param *pb.ReplaceArrayDeviceRequest_Param) (*pb.ReplaceArrayDeviceResponse, *pb.ReplaceArrayDeviceRequest, error) {
	command := "REPLACEARRAYDEVICE"
	req := &pb.ReplaceArrayDeviceRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendReplaceArrayDevice(p.connection, req)
	return res, req, err
}

// Reset the weights for backend events such as Flush, Rebuild, and GC to the default values.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ResetEventWRRPolicy() (*pb.ResetEventWrrResponse, *pb.ResetEventWrrRequest, error) {
	command := "RESETEVENTWRRPOLICY"
	req := &pb.ResetEventWrrRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendResetEventWrrPolicyRpc(p.connection, req)
	return res, req, err
}

// Reset MBR information of PoseidonOS. Use this command when you need to remove the all the arrays and reset the states of the devices.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ResetMBR() (*pb.ResetMbrResponse, *pb.ResetMbrRequest, error) {
	command := "RESETMBR"
	req := &pb.ResetMbrRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendResetMbrRpc(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) StopRebuilding(param *pb.StopRebuildingRequest_Param) (*pb.StopRebuildingResponse, *pb.StopRebuildingRequest, error) {
	command := "STOPREBUILDING"
	req := &pb.StopRebuildingRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendStopRebuildingRpc(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) UpdateEventWRRPolicy(param *pb.UpdateEventWrrRequest_Param) (*pb.UpdateEventWrrResponse, *pb.UpdateEventWrrRequest, error) {
	command := "UPDATEEVENTWRRPOLICY"
	req := &pb.UpdateEventWrrRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendUpdatEventWrr(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) DumpMemorySnapshot(param *pb.DumpMemorySnapshotRequest_Param) (*pb.DumpMemorySnapshotResponse, *pb.DumpMemorySnapshotRequest, error) {
	command := "DUMPMEMORYSNAPSHOT"
	req := &pb.DumpMemorySnapshotRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendDumpMemorySnapshot(p.connection, req)
	return res, req, err
}

// Apply a filtering policy to logger.
//Filtering file: when executing this command, PoseidonOS reads a filtering policy stored in a file. You can set the file path of the filter in the PoseidonOS configuration (the default path is /etc/conf/filter). If the file does not exist, you can create one.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ApplyLogFilter() (*pb.ApplyLogFilterResponse, *pb.ApplyLogFilterRequest, error) {
	command := "APPLYLOGFILTER"
	req := &pb.ApplyLogFilterRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendApplyLogFilter(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) GetLogLevel() (*pb.GetLogLevelResponse, *pb.GetLogLevelRequest, error) {
	command := "GETLOGLEVEL"
	req := &pb.GetLogLevelRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendGetLogLevel(p.connection, req)
	return res, req, err
}

// Display the current preference of logger
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) LoggerInfo() (*pb.LoggerInfoResponse, *pb.LoggerInfoRequest, error) {
	command := "LOGGERINFO"
	req := &pb.LoggerInfoRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendLoggerInfo(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) SetLogLevel(param *pb.SetLogLevelRequest_Param) (*pb.SetLogLevelResponse, *pb.SetLogLevelRequest, error) {
	command := "SETLOGLEVEL"
	req := &pb.SetLogLevelRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSetLogLevel(p.connection, req)
	return res, req, err
}

// Set the preferences (e.g., format) of logger.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) SetLogPreference(param *pb.SetLogPreferenceRequest_Param) (*pb.SetLogPreferenceResponse, *pb.SetLogPreferenceRequest, error) {
	command := "SETLOGPREFERENCE"
	req := &pb.SetLogPreferenceRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSetLogPreference(p.connection, req)
	return res, req, err
}

// Add a listener to an NVMe-oF subsystem
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) AddListener(param *pb.AddListenerRequest_Param) (*pb.AddListenerResponse, *pb.AddListenerRequest, error) {
	command := "ADDLISTENER"
	req := &pb.AddListenerRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendAddListener(p.connection, req)
	return res, req, err
}

// Remove a listener to an NVMe-oF subsystem
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) RemoveListener(param *pb.RemoveListenerRequest_Param) (*pb.RemoveListenerResponse, *pb.RemoveListenerRequest, error) {
	command := "REMOVELISTENER"
	req := &pb.RemoveListenerRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendRemoveListener(p.connection, req)
	return res, req, err
}

// List a listener to an NVMe-oF subsystem
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) ListListener(param *pb.ListListenerRequest_Param) (*pb.ListListenerResponse, *pb.ListListenerRequest, error) {
	command := "LISTLISTENER"
	req := &pb.ListListenerRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendListListener(p.connection, req)
	return res, req, err
}

// Set a listener's ana state to an NVMe-oF subsystem
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) SetListenerAnaState(param *pb.SetListenerAnaStateRequest_Param) (*pb.SetListenerAnaStateResponse, *pb.SetListenerAnaStateRequest, error) {
	command := "SETLISTENERANASTATE"
	req := &pb.SetListenerAnaStateRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSetListenerAnaState(p.connection, req)
	return res, req, err
}

// Create an NVMe-oF subsystem to PoseidonOS.
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) CreateSubsystem(param *pb.CreateSubsystemRequest_Param) (*pb.CreateSubsystemResponse, *pb.CreateSubsystemRequest, error) {
	command := "CREATESUBSYSTEM"
	req := &pb.CreateSubsystemRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateSubsystem(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) CreateSubsystemAuto(param *pb.CreateSubsystemRequest_Param) (*pb.CreateSubsystemResponse, *pb.CreateSubsystemRequest, error) {
	command := "CREATESUBSYSTEMAUTO"
	req := &pb.CreateSubsystemRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateSubsystem(p.connection, req)
	return res, req, err
}

// Create NVMf transport to PoseidonOS
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) CreateTransport(param *pb.CreateTransportRequest_Param) (*pb.CreateTransportResponse, *pb.CreateTransportRequest, error) {
	command := "CREATETRANSPORT"
	req := &pb.CreateTransportRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateTransport(p.connection, req)
	return res, req, err
}

// Delete a subsystem from PoseidonOS
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) DeleteSubsystem(param *pb.DeleteSubsystemRequest_Param) (*pb.DeleteSubsystemResponse, *pb.DeleteSubsystemRequest, error) {
	command := "DELETESUBSYSTEM"
	req := &pb.DeleteSubsystemRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendDeleteSubsystem(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) SubsystemInfo(param *pb.SubsystemInfoRequest_Param) (*pb.SubsystemInfoResponse, *pb.SubsystemInfoRequest, error) {
	command := "SUBSYSTEMINFO"
	req := &pb.SubsystemInfoRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSubsystemInfo(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ListSubsystem() (*pb.ListSubsystemResponse, *pb.ListSubsystemRequest, error) {
	command := "LISTSUBSYSTEM"
	req := &pb.ListSubsystemRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendListSubsystem(p.connection, req)
	return res, req, err
}

// Start the collection of telemetry data in PoseidonOS
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) StartTelemetry() (*pb.StartTelemetryResponse, *pb.StartTelemetryRequest, error) {
	command := "STARTTELEMETRY"
	req := &pb.StartTelemetryRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendStartTelemetryRpc(p.connection, req)
	return res, req, err
}

// Stop the collection of telemetry data in PoseidonOS
// The function takes a protobuf format as parameter and returns response in protobuf format
func (p *POSGRPCManager) StopTelemetry() (*pb.StopTelemetryResponse, *pb.StopTelemetryRequest, error) {
	command := "STOPTELEMETRY"
	req := &pb.StopTelemetryRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendStopTelemetryRpc(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) CreateVolume(param *pb.CreateVolumeRequest_Param) (*pb.CreateVolumeResponse, *pb.CreateVolumeRequest, error) {
	command := "CREATEVOLUME"
	req := &pb.CreateVolumeRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateVolume(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) SetTelemetryProperty(param *pb.SetTelemetryPropertyRequest_Param) (*pb.SetTelemetryPropertyResponse, *pb.SetTelemetryPropertyRequest, error) {
	command := "SETTELEMETRYPROPERTY"
	req := &pb.SetTelemetryPropertyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendSetTelemetryPropertyRpc(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) GetTelemetryProperty() (*pb.GetTelemetryPropertyResponse, *pb.GetTelemetryPropertyRequest, error) {
	command := "GETTELEMETRYPROPERTY"
	req := &pb.GetTelemetryPropertyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
	res, err := grpc.SendGetTelemetryProperty(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) RebuildArray(param *pb.RebuildArrayRequest_Param) (*pb.RebuildArrayResponse, *pb.RebuildArrayRequest, error) {
	command := "REBUILDARRAY"
	req := &pb.RebuildArrayRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendRebuildArray(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) VolumeProperty(param *pb.SetVolumePropertyRequest_Param) (*pb.SetVolumePropertyResponse, *pb.SetVolumePropertyRequest, error) {
	command := "SETVOLUMEPROPERTY"
	req := &pb.SetVolumePropertyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendVolumeProperty(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) DeleteVolume(param *pb.DeleteVolumeRequest_Param) (*pb.DeleteVolumeResponse, *pb.DeleteVolumeRequest, error) {
	command := "DELETEVOLUME"
	req := &pb.DeleteVolumeRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendDeleteVolume(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) MountVolume(param *pb.MountVolumeRequest_Param) (*pb.MountVolumeResponse, *pb.MountVolumeRequest, error) {
	command := "MOUNTVOLUME"
	req := &pb.MountVolumeRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendMountVolume(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) UnmountVolume(param *pb.UnmountVolumeRequest_Param) (*pb.UnmountVolumeResponse, *pb.UnmountVolumeRequest, error) {
	command := "UNMOUNTVOLUME"
	req := &pb.UnmountVolumeRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendUnmountVolume(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ListVolume(param *pb.ListVolumeRequest_Param) (*pb.ListVolumeResponse, *pb.ListVolumeRequest, error) {
	command := "LISTVOLUME"
	req := &pb.ListVolumeRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendListVolume(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) VolumeInfo(param *pb.VolumeInfoRequest_Param) (*pb.VolumeInfoResponse, *pb.VolumeInfoRequest, error) {
	command := "VOLUMEINFO"
	req := &pb.VolumeInfoRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendVolumeInfo(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) VolumeRename(param *pb.VolumeRenameRequest_Param) (*pb.VolumeRenameResponse, *pb.VolumeRenameRequest, error) {
	command := "RENAMEVOLUME"
	req := &pb.VolumeRenameRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendVolumeRename(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) CreateQoSVolumePolicy(param *pb.QosCreateVolumePolicyRequest_Param) (*pb.QosCreateVolumePolicyResponse, *pb.QosCreateVolumePolicyRequest, error) {
	command := "CREATEQOSVOLUMEPOLICY"
	req := &pb.QosCreateVolumePolicyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendCreateQoSVolumePolicy(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ResetQoSVolumePolicy(param *pb.QosResetVolumePolicyRequest_Param) (*pb.QosResetVolumePolicyResponse, *pb.QosResetVolumePolicyRequest, error) {
	command := "RESETQOSVOLUMEPOLICY"
	req := &pb.QosResetVolumePolicyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendResetQoSVolumePolicy(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ListQoSVolumePolicy(param *pb.ListQOSPolicyRequest_Param) (*pb.ListQOSPolicyResponse, *pb.ListQOSPolicyRequest, error) {
	command := "LISTQOSPOLICIES"
	req := &pb.ListQOSPolicyRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendListQoSPolicy(p.connection, req)
	return res, req, err
}

func (p *POSGRPCManager) ListWBT() (*pb.ListWBTResponse, *pb.ListWBTRequest, error) {
    command := "LISTWBT"
    req := &pb.ListWBTRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor}
    res, err := grpc.SendListWBT(p.connection, req)
    return res, req, err
}

func (p *POSGRPCManager) WBT(param *pb.WBTRequest_Param) (*pb.WBTResponse, *pb.WBTRequest, error) {
    command := "WBT"
    req := &pb.WBTRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
    res, err := grpc.SendWBT(p.connection, req)
    return res, req, err
}


func (p *POSGRPCManager) WithTimeout(timeout uint32) (POSManager) {
    grpcManagerWithTimeout := *p
    grpcManagerWithTimeout.connection.ReqTimeout = timeout
    grpcManagerWithTimeout.connection.TimeoutChanged = true
    return &grpcManagerWithTimeout
}

