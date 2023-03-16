# Add a New CLI Command
To Add a new CLI client command corresponding to a server command in PoseidonOS, the following steps should be done:
1. Create an interface function in Kouros
2. Implement the interface function in Kouros
3. Call the function in CLI command

Below, the steps for adding the command are explained.

Before creating the command, the PoseidonOS should be cloned and available. Follow the below steps for PoseidonOS cloning:
`
git clone https://github.com/poseidonos/poseidonos.git
cd poseidonos
`
The below steps mention all the paths relative to poseidonos directory which is created by cloning in the above step.
## 1. Create an Interface Function in Kouros

The `tool/kouros/pos/pos_manager.go` file contains all the functions that will be supported by the cli grpc server. The PoseidonOS CLI Server will have a .proto file associated with it. The new function should be using the Request and Response structure defined in the .proto file.

For eg:
Suppose, a function NewTestFunction is going to be implemented, then the function needs to be added in the POSManager interface inside the `tool/kouros/pos/pos_manager.go` file like below.
```
NewTestFunction(param *pb.NewTestRequest) (response *pb.NewTestResponse, request *pb.NewTestRequest, err error)
```

where pb.NewTestRequest corresponds to the NewTestRequest structure and pb.NewTestResponse corresponds to the NewTestResponse structure in cli.proto file

## 2. Implement the interface function in Kouros

After adding the NewTestFunction inside the interface, the function should be implemented in the POSGRPCManager struct `tool/kouros/pos/grpc_manager.go`

The implementation will look like the one below:

```
func (p *POSGRPCManager) NewTest(param *pb.NewTestRequest_Param) (*pb.NewTestResponse, *pb.NewTestRequest, error) {
	command := "NEWTEST"
	req := &pb.NewTestRequest{Command: command, Rid: utils.GenerateUUID(), Requestor: p.requestor, Param: param}
	res, err := grpc.SendNewTest(p.connection, req)
	return res, req, err
}
```

In the above code, pb.NewTestRequest corresponds to the NewTestRequest structure, pb.NewTestResponse corresponds to the NewTestResponse structure and pb.NewTestRequest_Param corresponds to the NewTestRequest_Param structure in cli.proto file.
command is a name given to the command for identification in PoseidonOS
grpc.SendNewTest is a function that will take the connection in POSGRPCManager structure and request as parameters and send it to the grpc server. For adding the SendNewTest command, follow the below instructions.

Implement SendNewTest function inside `tool/kouros/pos/grpc/grpc_service.go`. The function can be implemented as below:

```
func SendNewTestposConn POSGRPCConnection, req *pb.StopSystemRequest) (*pb.StopSystemResponse, error) {
	conn, err := dialToCliServer(posConn)
	if err != nil {
		log.Print(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(posConn.ReqTimeout))
	defer cancel()

	res, err := c.NewTest(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}
```

In the above code, c.NewTest will call the CLI Server and get the response.

## 3. Call the function in CLI command

While creating a new command in CLI, the kouros module should be imported in the file where command is implemented.

The Run or RUnE function of the command corresponding to the implementation of command in above example can be similar to the one given below:

```
RunE: func(cmd *cobra.Command, args []string) error {
    param := &pb.NewTestRequest_Param{TestParam: test_param}      // Creating request parameter. test_param is the parameter extracted from the command

	posMgr, err := grpcmgr.GetPOSManager()                        // Obtain the Kouros POSManager. The grpcmgr.GetPOSManager is already available in the cli code
	if err != nil {
		fmt.Printf("failed to connect to POS: %v", err)
		return err
	}
	res, req, gRpcErr := posMgr.NewTest(reqParam)                 // Calling the Kouros function implemented in the examples above

	reqJson, err := protojson.MarshalOptions{                     // Converting request to JSON for displaying in CLI output
		EmitUnpopulated: true,
	}.Marshal(req)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf request: %v", err)
		return err
	}
	displaymgr.PrintRequest(string(reqJson))                      // Displaying the JSON request if --json flag is set in CLI command

	if gRpcErr != nil {
		globals.PrintErrMsg(gRpcErr)
		return gRpcErr
	}

	printErr := displaymgr.PrintProtoResponse(req.Command, res)       // Printing the Response of the command
	if printErr != nil {
		fmt.Printf("failed to print the response: %v", printErr)
		return printErr
	}

	return nil
}

```


