package grpcmgr

import (
	"cli/cmd/globals"
	"cli/cmd/otelmgr"
	"context"
	"errors"
	"fmt"
	"pnconnector/src/log"
	"time"

    "kouros"
    "kouros/pos"
	pb "kouros/api"

	"google.golang.org/grpc"
)

const dialErrorMsg = "Could not connect to the CLI server. Is PoseidonOS running?"
const dialTimeout = 10

// TODO (mj): We temporarily set long timeout values for mount/unmount array commands.
const (
	unmountArrayCmdTimeout uint32 = 1800
	mountArrayCmdTimeout   uint32 = 600
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

func dialToCliServer() (*grpc.ClientConn, error) {
	nodeName := globals.NodeName
	gRpcServerAddress := globals.GrpcServerAddress

	if nodeName != "" {
		var err error
		gRpcServerAddress, err = GetIpv4(nodeName)
		if err != nil {
			return nil, errors.New("an error occured while getting the ipv4 address of a node: " + err.Error())
		}
	}

	conn, err := grpc.Dial(gRpcServerAddress, grpc.WithTimeout(time.Second*dialTimeout), grpc.WithInsecure(), grpc.WithBlock())
	return conn, err
}

func SendSystemInfo(ctx context.Context, req *pb.SystemInfoRequest) (*pb.SystemInfoResponse, error) {
	t := otelmgr.NewTracer()
	t.SetTrace(ctx, globals.GRPC_MGR_APP_NAME, globals.GRPC_SYSTEM_INFO_FUNC_NAME)
	defer t.Release()

	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		t.RecordError(err)
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.SystemInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		t.RecordError(err)
		return nil, err
	}

	return res, err
}

func SendStopSystem(req *pb.StopSystemRequest) (*pb.StopSystemResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.StopSystem(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetSystemProperty(req *pb.GetSystemPropertyRequest) (*pb.GetSystemPropertyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.GetSystemProperty(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetSystemProperty(req *pb.SetSystemPropertyRequest) (*pb.SetSystemPropertyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.SetSystemProperty(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStartTelemetryRpc(req *pb.StartTelemetryRequest) (*pb.StartTelemetryResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.StartTelemetry(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopTelemetryRpc(req *pb.StopTelemetryRequest) (*pb.StopTelemetryResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.StopTelemetry(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetTelemetryPropertyRpc(req *pb.SetTelemetryPropertyRequest) (*pb.SetTelemetryPropertyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.SetTelemetryProperty(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetTelemetryProperty(req *pb.GetTelemetryPropertyRequest) (*pb.GetTelemetryPropertyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.GetTelemetryProperty(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateSubsystem(req *pb.CreateSubsystemRequest) (*pb.CreateSubsystemResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.CreateSubsystem(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteSubsystem(req *pb.DeleteSubsystemRequest) (*pb.DeleteSubsystemResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.DeleteSubsystem(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAddListener(req *pb.AddListenerRequest) (*pb.AddListenerResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.AddListener(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListSubsystem(req *pb.ListSubsystemRequest) (*pb.ListSubsystemResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.ListSubsystem(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSubsystemInfo(req *pb.SubsystemInfoRequest) (*pb.SubsystemInfoResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.SubsystemInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateTransport(req *pb.CreateTransportRequest) (*pb.CreateTransportResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.CreateTransport(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateVolume(req *pb.CreateVolumeRequest) (*pb.CreateVolumeResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.CreateVolume(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteVolume(req *pb.DeleteVolumeRequest) (*pb.DeleteVolumeResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.DeleteVolume(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendMountVolume(req *pb.MountVolumeRequest) (*pb.MountVolumeResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.MountVolume(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUnmountVolume(req *pb.UnmountVolumeRequest) (*pb.UnmountVolumeResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.UnmountVolume(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeProperty(req *pb.SetVolumePropertyRequest) (*pb.SetVolumePropertyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.SetVolumeProperty(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListVolume(req *pb.ListVolumeRequest) (*pb.ListVolumeResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.ListVolume(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendQosCreateVolumePolicy(req *pb.QosCreateVolumePolicyRequest) (*pb.QosCreateVolumePolicyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()
	res, err := c.QosCreateVolumePolicy(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeInfo(req *pb.VolumeInfoRequest) (*pb.VolumeInfoResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()
	res, err := c.VolumeInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendQosResetVolumePolicy(req *pb.QosResetVolumePolicyRequest) (*pb.QosResetVolumePolicyResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()
	res, err := c.QosResetVolumePolicy(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeRename(req *pb.VolumeRenameRequest) (*pb.VolumeRenameResponse, error) {
    conn, err := dialToCliServer()
    if err != nil {
        log.Error(err)
        errToReturn := errors.New(dialErrorMsg)
        return nil, errToReturn
    }
    defer conn.Close()

    c := pb.NewPosCliClient(conn)
    ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
    defer cancel()
    res, err := c.VolumeRename(ctx, req)
    if err != nil {
        log.Error("error: ", err.Error())
        return nil, err
    }

    return res, err

}

func SendListWBT(req *pb.ListWBTRequest) (*pb.ListWBTResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.ListWBT(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListQOSPolicy(req *pb.ListQOSPolicyRequest) (*pb.ListQOSPolicyResponse, error) {
    conn, err := dialToCliServer()
    if err != nil {
        log.Error(err)
        errToReturn := errors.New(dialErrorMsg)
        return nil, errToReturn
    }
    defer conn.Close()

    c := pb.NewPosCliClient(conn)
    ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
    defer cancel()
    res, err := c.ListQOSPolicy(ctx, req)
    if err != nil {
        log.Error("error: ", err.Error())
        return nil, err
    }

    return res, err
}

func SendWBT(req *pb.WBTRequest) (*pb.WBTResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.WBT(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}
