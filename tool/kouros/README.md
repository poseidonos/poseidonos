# kouros
--
    import "."

Package kouros implements functions that can be used by clients developed in go
to access the poseidonos and give commands to PoseidonOS and 2-Node HA

## Usage

#### func  NewHAManager

```go
func NewHAManager(managerType string) (ha.HAManager, error)
```
NewPOSManager creates a new manager object based on the type provided Currently
only type "postgres" is supported

#### func  NewPOSManager

```go
func NewPOSManager(managerType string) (pos.POSManager, error)
```
NewPOSManager creates a new manager object based on the type provided Currently
only type "grpc" is supported
