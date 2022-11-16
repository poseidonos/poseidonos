```mermaid
classDiagram
    class POSInterface {
        <<enumeration>>
        GRPC
    }
    class HAInterface {
        <<enumeration>>
        GRPC
    }
    class Kouros {
        <<package>>
        NewPOSManager(interfaceType: POSInterface) POSManager
        NewHAManager(interfaceType: HAInterface) HAManager
    }
    class POSManager {
        <<interface>>
        Init(string, any): error
        CreateArray(CreateArrayRequest_Param): (CreateArrayResponse, CreateArrayRequest, error)
        CreateVolume (CreateVolumeRequest_Param): (CreateVolumeResponse, CreateVolumeRequest, error)
        DeleteArray(DeleteArrayRequest_Param): (DeleteArrayResponse, DeleteArrayRequest, error)
        DeleteVolume(DeleteVolumeRequest_Param): (DeleteVolumeResponse, DeleteVolumeRequest, error)
    }
    class HAManager {
        <<interface>>
        Init(string, any): error
        listNodes(): (ListNodeResponse, ListNodeRequest, error)
        listVolumes(): (ListVolumesResponse, ListVolumesRequest, error)
        listReplication(): (ListReplicationResponse, ListReplicationRequest, error)
        startReplication(): (StartReplicationResponse, StartReplicationRequest, error)
        stopReplication(): (StopReplicationResponse, StopReplicationRequest, error)
    }
    class POSGRPCManager {
        connection: POSGRPCConnection
        requestor: string
    }

    class HAPostgresManager {
        connection: PostgresConnection
    }
    class CreateArrayResponse {
        <<proto message>>
        rid: string
        command: string
        result: Result
        info: POSInfo
    }
    class Result {
        <<proto message>>
        status: Status
    }
    class POSInfo {
        <<proto message>>
        version: string
    }
    class Status {
        <<proto message>>
        code: int
        event_name: string
        description: string
        cause: string
        solution: string
    }
    class CreateArrayRequest {
        <<proto message>>
        command: string
        rid: string
        requestor: string
        Param: CreateArrayRequest_Param
    }
    class CreateArrayRequest_Param {
        <<proto message>>
        name: string
        raidtype: string
        buffer: []Device
        data: []Device
        spare: []Device
    }
    class Device {
        <<proto message>>
        deviceName: string
    }
    class POSGRPCConnection {
        address: string
    }
    class PostgresConnection {
        url: string
        username: string
        password: string
    }



    class GRPCService {
        dial() (*grpc.ClientConn, err)
        createArray(CreateArrayRequest) (CreateArrayResponse, Error)
        createVolume(CreateVolumeRequest) (CreateVolumeResponse, Error))
        deleteArray(DeleteArrayRequest) (DeleteArrayResponse, Error))
        deleteVolume(DeleteVolumeRequest) (DeleteVolumeResponse, Error))
    }

    class PostgresDBService {
        connect() (*sql.DB, error)
        listNodes(ListNodeRequest): (ListNodeResponse, Error)
        listVolumes(ListHAVolumeRequest): (ListHAVolumeResponse, Error)
        listReplication(ListHAReplicationRequest): (ListHAReplicationResponse, Error)
        startReplication(startReplicationRequest): (startReplicationResponse, Error)
        stopReplication(stopReplicationRequest): (stopReplicationResponse, Error)
    }



    POSManager <|-- POSGRPCManager : implements
    HAManager <|-- HAPostgresManager : implements
    Status --* Result : Composition
    Result --* CreateArrayResponse : Composition
    CreateArrayResponse --> GRPCService
    Device --o CreateArrayRequest_Param
    CreateArrayRequest_Param --* CreateArrayRequest
    POSInfo --* Result
    CreateArrayRequest --> GRPCService
    POSGRPCConnection --* POSGRPCManager
    PostgresConnection --* HAPostgresManager
    POSGRPCManager --> GRPCService : uses
    HAPostgresManager --> PostgresDBService : uses
    POSManager --> CreateArrayRequest_Param : uses
    POSManager --> CreateArrayRequest : uses
    POSManager --> CreateArrayResponse : uses
    Kouros --> POSInterface : uses
    Kouros --> HAInterface : uses
    Kouros --> POSManager : uses
    Kouros --> HAManager : uses
```

The above class diagram for Kuoros module shows how the Kuoros module will be structured.
The POSManager interface will have all the commands that can be executed on POS and HAManager will have all the commands that can be executed on HA DB.

The above class Diagram also represents the Command for CreateArray function.

For a client application, to use the Kuoros module, the following code should be used:
```
posConnector := kouros.NewPOSManager("grpc")
posGRPCConnection := kouros.POSGRPCConnection{
        addres: "127.0.0.1"
}
posConnector.init(posGRPCConnection);

posMngr, _ := kouros.NewPOSManager(pos.GRPC)
posMngr.Init("CLI", globals.GrpcServerAddress)

createArrayParam := &api.CreateArrayRequest_Param{
        Name: "POSArray",
        Raidtype: "RAID0",
        Buffer: []*pb.DeviceNameList{
            &pb.DeviceNameList{
                DeviceName: "uram0",
            },
        },
        Data: []*pb.DeviceNameList{
            &pb.DeviceNameList{
                DeviceName: "unvme-ns-0",
            },
        },
        Spare: []*pb.DeviceNameList{
            &pb.DeviceNameList{
                DeviceName: "unvme-ns-1",
            },
        },
    }


res, req, err := posMngr.CreateArray(createArrayParam)

if err != nil {
    log.Fatalf("Error in Array Creation: %v", err)
}

```

Similarly user can create an object for HAManager and use it to call HA commands

For Adding a new POS function, the following steps need to be done:
1. Create a message structure for GRPC Protobuf message
2. Add the function signature to POSManager
3. Implement the function in POSGRPCManager
