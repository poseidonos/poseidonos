```mermaid
classDiagram
    class Kouros {
        <<package>>
        NewPOSManager(string) POSManager
        NewHAManager(string) HAManager
    }
    class POSManager {
        <<interface>>
        createArray([]byte): ([]byte, Error)
        createVolume ([]byte): ([]byte, Error)
        deleteArray([]byte): ([]byte, Error)
        deleteVolume([]byte): ([]byte, Error)
    }
    class HAManager {
        <<interface>>
        listNodes(): ([]byte, Error)
        listVolumes(): ([]byte, Error)
        listReplication(): ([]byte, Error)
        startReplication([]byte): ([]byte, Error)
        stopReplication([]byte): ([]byte, Error)
    }
    class POSGRPCManager {
        connection: POSGRPCConnection
        init(POSGRPCConnection)
        connect(POSGRPCConnection) Error
    }

    class HAPostgresManager {
        connection: PostgresConnection
        init(POSGRPCConnection)
        connect(PostgresConnection) Error
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
    class CreateArrayCommand {
        <<proto message>>
        command: string
        rid: string
        requestor: string
        Param: CreateArrayParam
    }
    class CreateArrayParam {
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
        createArray(*pb.CreateArrayRequest) (*pb.CreateArrayResponse, Error)
        createVolume(*pb.CreateVolumeRequest) (*pb.CreateVolumeResponse, Error))
        deleteArray(*pb.DeleteArrayRequest) (*pb.DeleteArrayResponse, Error))
        deleteVolume(*pb.DeleteVolumeRequest) (*pb.DeleteVolumeResponse, Error))
    }

    class PostgresDBService {
        connect() (*sql.DB, error)
        listNodes(*pb.ListNodeRequest): (*pb.ListNodeResponse, Error)
        listVolumes(*pb.ListHAVolumeRequest): (*pb.ListHAVolumeResponse, Error)
        listReplication(*pb.ListHAReplicationRequest): (*pb.ListHAReplicationResponse, Error)
        startReplication(*pb.startReplicationRequest): (*pb.startReplicationResponse, Error)
        stopReplication(*pb.stopReplicationRequest): (*pb.stopReplicationResponse, Error)
    }



    POSManager <|-- POSGRPCManager : implements
    HAManager <|-- HAPostgresManager : implements
    Status --* Result : Composition
    Result --* CreateArrayResponse : Composition
    CreateArrayResponse --> GRPCService
    Device --o CreateArrayParam
    CreateArrayParam --* CreateArrayCommand
    POSInfo --* Result
    CreateArrayCommand --> GRPCService
    POSGRPCConnection --* POSGRPCManager
    PostgresConnection --* HAPostgresManager
    POSGRPCManager --> GRPCService : uses
    HAPostgresManager --> PostgresDBService : uses
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



createArrayCommand := []byte(`{
    param: {
        "name": "POSArray",
        "raidtype": "RAID0",
        "buffer": {
            "device": {
                "deviceName": "uram0"
            }
        }
        "data": {
            {
                deviceName: "unvme-ns-0"
            }
        }
        "spare": {
            {
                deviceName: "unvme-ns-1"
            }
        }
    }
}`)

res, err := posConnector.CreateArray(createArrayCommand)
if err != nil {
    log.Fatalf("Error in Array Creation: %v", err)
}
log.Println(string(res))
```

Similarly user can create an object for HAManager and use it to call HA commands

For Adding a new POS function, the following steps need to be done:
1. Create a message structure for GRPC Protobuf message
2. Add the function signature to POSManager
3. Implement the function in POSGRPCManager

