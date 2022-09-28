package grpcmgr

import (
	"cli/cmd/globals"
	"cli/cmd/otelmgr"
	"context"
	"errors"
	"fmt"
	"pnconnector/src/log"
	"time"

	pb "cli/api"

	"google.golang.org/grpc"
)

const dialErrorMsg = "Could not connect to the CLI server. Is PoseidonOS running?"
const dialTimeout = 10

// TODO (mj): temporarily set the timeout for unmount array command to 30 minutes
// because the command exceptionally takes too long.
// However, the timeout needs to be systemically set in the future.
const unmountArrayCmdTimeout = 1800

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

func SendResetEventWrrPolicyRpc(req *pb.ResetEventWrrRequest) (*pb.ResetEventWrrResponse, error) {
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

	res, err := c.ResetEventWrr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetMbrRpc(req *pb.ResetMbrRequest) (*pb.ResetMbrResponse, error) {
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

	res, err := c.ResetMbr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopRebuildingRpc(req *pb.StopRebuildingRequest) (*pb.StopRebuildingResponse, error) {
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

	res, err := c.StopRebuilding(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUpdatEventWrr(req *pb.UpdateEventWrrRequest) (*pb.UpdateEventWrrResponse, error) {
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

	res, err := c.UpdateEventWrr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAddSpare(req *pb.AddSpareRequest) (*pb.AddSpareResponse, error) {
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

	res, err := c.AddSpare(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendRemoveSpare(req *pb.RemoveSpareRequest) (*pb.RemoveSpareResponse, error) {
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

	res, err := c.RemoveSpare(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendReplaceArrayDevice(req *pb.ReplaceArrayDeviceRequest) (*pb.ReplaceArrayDeviceResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithTimeout(time.Second*dialTimeout), grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(globals.ReqTimeout))
	defer cancel()

	res, err := c.ReplaceArrayDevice(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateArray(req *pb.CreateArrayRequest) (*pb.CreateArrayResponse, error) {
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

	res, err := c.CreateArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAutocreateArray(req *pb.AutocreateArrayRequest) (*pb.AutocreateArrayResponse, error) {
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

	res, err := c.AutocreateArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteArray(req *pb.DeleteArrayRequest) (*pb.DeleteArrayResponse, error) {
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

	res, err := c.DeleteArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendMountArray(req *pb.MountArrayRequest) (*pb.MountArrayResponse, error) {
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

	res, err := c.MountArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUnmountArray(req *pb.UnmountArrayRequest) (*pb.UnmountArrayResponse, error) {
	conn, err := dialToCliServer()
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(unmountArrayCmdTimeout))
	defer cancel()

	res, err := c.UnmountArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendArrayInfo(req *pb.ArrayInfoRequest) (*pb.ArrayInfoResponse, error) {
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

	res, err := c.ArrayInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendRebuildArray(req *pb.RebuildArrayRequest) (*pb.RebuildArrayResponse, error) {
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

	res, err := c.RebuildArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListArray(req *pb.ListArrayRequest) (*pb.ListArrayResponse, error) {
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

	res, err := c.ListArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetLogPreference(req *pb.SetLogPreferenceRequest) (*pb.SetLogPreferenceResponse, error) {
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

	res, err := c.SetLogPreference(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetLogLevel(req *pb.SetLogLevelRequest) (*pb.SetLogLevelResponse, error) {
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

	res, err := c.SetLogLevel(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendLoggerInfo(req *pb.LoggerInfoRequest) (*pb.LoggerInfoResponse, error) {
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

	res, err := c.LoggerInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetLogLevel(req *pb.GetLogLevelRequest) (*pb.GetLogLevelResponse, error) {
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

	res, err := c.GetLogLevel(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendApplyLogFilter(req *pb.ApplyLogFilterRequest) (*pb.ApplyLogFilterResponse, error) {
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

	res, err := c.ApplyLogFilter(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateDevice(req *pb.CreateDeviceRequest) (*pb.CreateDeviceResponse, error) {
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

	res, err := c.CreateDevice(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendScanDevice(req *pb.ScanDeviceRequest) (*pb.ScanDeviceResponse, error) {
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

	res, err := c.ScanDevice(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListDevice(req *pb.ListDeviceRequest) (*pb.ListDeviceResponse, error) {
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

	res, err := c.ListDevice(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetSmartLog(req *pb.GetSmartLogRequest) (*pb.GetSmartLogResponse, error) {
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

	res, err := c.GetSmartLog(ctx, req)
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
