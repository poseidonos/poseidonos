```mermaid
classDiagram
    class POSManager {
        <<interface>>
        createArray(CreateArrayCommand): (Response, Error)
        createVolume (CreateVolumeCommand): (Response, Error)
        deleteArray(DeleteArrayCommand): (Response, Error)
        deleteVolume(DeleteVolumeCommand): (Response, Error)
    }
    class HAManager {
        <<interface>>
        listNodes(): (Response, Error)
        listVolumes(): (Response, Error)
        listReplication(): (Response, Error)
        startReplication(StartReplicationCommand): (Response, Error)
        stopReplication(StopReplicationCommand): (Response, Error)
    }
    class POSGRPCManager {
        connection: GRPCConnection
    }

    class HAPostgresManager {
        connection: PostgresConnection
    }
    class Response {
        rid: string
        lastSuccessTime: int
        result: Result
        info: any
    }
    class Result {
        status: Status
        data: any
    }
    class Status {
        level: string
        description: string
        problem: string
        solution: string
    }
    class Command {
        command: string
        rid: string
        requestor: string
    }
    class CreateArrayCommand {
        Param: CreateArrayParam
    }
    class CreateArrayParam {
        name: string
        raidtype: string
        buffer: []Device
        data: []Device
        spare: []Device
    }
    class Device {
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

    POSManager <|-- POSGRPCManager : implements
    HAManager <|-- HAPostgresManager : implements
    Status --* Result : Composition
    Result --* Response : Composition
    Response --> POSManager : Association
    Response --> HAManager : Association
    Device --o CreateArrayParam
    CreateArrayParam --* CreateArrayCommand 
    CreateArrayCommand --|> Command : Inheritance
    CreateArrayCommand -->POSManager
    POSGRPCConnection --* POSGRPCManager
    PostgresConnection --* HAPostgresManager

```

The above class diagram for Kuoros module shows how the Kuoros module will be structured.
The POSManager interface will have all the commands that can be executed on POS and HAManager will have all the commands that can be executed on HA DB.

The above class Diagram also represents the Command for CreateArray function.

For a client application, to use the Kuoros module, the following code should be used:
```
posConnector := POSGRPCManager{
    POSGRPCConnection{
        addres: "127.0.0.1"
    }
}

createArrayCommand := CreateArrayCommand{
    Param: {
        name: "POSArray",
        raidtype: "RAID0",
        buffer: []Device{
            Device{
                deviceName: "uram0"
            }
        }
        data: []Device{
            Device{
                deviceName: "unvme-ns-0"
            }
        }
        spare: []Device{
            Device{
                deviceName: "unvme-ns-1"
            }
        }
    }
}

res, err := posConnector.CreateArray(createArrayCommand)
```

Similarly user can create an object for HAManager and use it to call HA commands

For Adding a new POS function, the following steps need to be done:
1. Command structure need to be defined similar to CreateArrayCommand 
2. Add the function signature to POSManager
3. Implement the function in POSGRPCManager

