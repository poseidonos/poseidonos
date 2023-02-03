package grpcmgr

import (
	"cli/cmd/globals"
	"errors"

    "kouros"
    "kouros/pos"
)

func GetPOSManager() (pos.POSManager, error) {
    posMngr, err := kouros.NewPOSManager(pos.GRPC)
    if err != nil {
        return nil, err
    }

    nodeName := globals.NodeName
    gRpcServerAddress := globals.GrpcServerAddress

    if nodeName != "" {
        var err error
        gRpcServerAddress, err = GetIpv4(nodeName)
        if err != nil {
            return nil, errors.New("an error occured while getting the ipv4 address of a node: " + err.Error())
        }
    }

    posMngr.Init("cli", gRpcServerAddress)
    return posMngr, err
}

