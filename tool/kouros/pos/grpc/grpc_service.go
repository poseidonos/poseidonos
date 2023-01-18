package grpc

import (
	"context"
	"errors"
	"fmt"
	"log"
	"time"

	pb "kouros/api"

	googleGRPC "google.golang.org/grpc"
)

const dialErrorMsg = "Could not connect to the CLI server. Is PoseidonOS running?"
const dialTimeout = 5

// TODO  We temporarily set long timeout values for mount/unmount array commands.
const unmountArrayCmdTimeout uint32 = 1800
const mountArrayCmdTimeout uint32 = 600


type POSGRPCConnection struct {
	Address string
    ReqTimeout uint32
    TimeoutChanged bool
}

func dialToCliServer(posConn POSGRPCConnection) (*googleGRPC.ClientConn, error) {

	conn, err := googleGRPC.Dial(posConn.Address, googleGRPC.WithTimeout(time.Second*dialTimeout), googleGRPC.WithInsecure(), googleGRPC.WithBlock())
	return conn, err
}

func SendSystemInfo(posConn POSGRPCConnection, req *pb.SystemInfoRequest) (*pb.SystemInfoResponse, error) {
	conn, err := dialToCliServer(posConn)
	if err != nil {
		err := errors.New(fmt.Sprintf("%s (internal error message: %s)",
			dialErrorMsg, err.Error()))
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(posConn.ReqTimeout))
	defer cancel()

	res, err := c.SystemInfo(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopSystem(posConn POSGRPCConnection, req *pb.StopSystemRequest) (*pb.StopSystemResponse, error) {
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

	res, err := c.StopSystem(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetSystemProperty(posConn POSGRPCConnection, req *pb.GetSystemPropertyRequest) (*pb.GetSystemPropertyResponse, error) {
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

	res, err := c.GetSystemProperty(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetSystemProperty(posConn POSGRPCConnection, req *pb.SetSystemPropertyRequest) (*pb.SetSystemPropertyResponse, error) {
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

	res, err := c.SetSystemProperty(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStartTelemetryRpc(posConn POSGRPCConnection, req *pb.StartTelemetryRequest) (*pb.StartTelemetryResponse, error) {
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

	res, err := c.StartTelemetry(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopTelemetryRpc(posConn POSGRPCConnection, req *pb.StopTelemetryRequest) (*pb.StopTelemetryResponse, error) {
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

	res, err := c.StopTelemetry(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetTelemetryPropertyRpc(posConn POSGRPCConnection, req *pb.SetTelemetryPropertyRequest) (*pb.SetTelemetryPropertyResponse, error) {
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

	res, err := c.SetTelemetryProperty(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetTelemetryProperty(posConn POSGRPCConnection, req *pb.GetTelemetryPropertyRequest) (*pb.GetTelemetryPropertyResponse, error) {
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

	res, err := c.GetTelemetryProperty(ctx, req)

	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetEventWrrPolicyRpc(posConn POSGRPCConnection, req *pb.ResetEventWrrRequest) (*pb.ResetEventWrrResponse, error) {
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

	res, err := c.ResetEventWrr(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetMbrRpc(posConn POSGRPCConnection, req *pb.ResetMbrRequest) (*pb.ResetMbrResponse, error) {
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

	res, err := c.ResetMbr(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopRebuildingRpc(posConn POSGRPCConnection, req *pb.StopRebuildingRequest) (*pb.StopRebuildingResponse, error) {
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

	res, err := c.StopRebuilding(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUpdatEventWrr(posConn POSGRPCConnection, req *pb.UpdateEventWrrRequest) (*pb.UpdateEventWrrResponse, error) {
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

	res, err := c.UpdateEventWrr(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDumpMemorySnapshot(posConn POSGRPCConnection, req *pb.DumpMemorySnapshotRequest) (*pb.DumpMemorySnapshotResponse, error) {
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

	res, err := c.DumpMemorySnapshot(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAddSpare(posConn POSGRPCConnection, req *pb.AddSpareRequest) (*pb.AddSpareResponse, error) {
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

	res, err := c.AddSpare(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendRemoveSpare(posConn POSGRPCConnection, req *pb.RemoveSpareRequest) (*pb.RemoveSpareResponse, error) {
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

	res, err := c.RemoveSpare(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendReplaceArrayDevice(posConn POSGRPCConnection, req *pb.ReplaceArrayDeviceRequest) (*pb.ReplaceArrayDeviceResponse, error) {
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

	res, err := c.ReplaceArrayDevice(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateArray(posConn POSGRPCConnection, req *pb.CreateArrayRequest) (*pb.CreateArrayResponse, error) {
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

	res, err := c.CreateArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAutocreateArray(posConn POSGRPCConnection, req *pb.AutocreateArrayRequest) (*pb.AutocreateArrayResponse, error) {
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

	res, err := c.AutocreateArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteArray(posConn POSGRPCConnection, req *pb.DeleteArrayRequest) (*pb.DeleteArrayResponse, error) {
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

	res, err := c.DeleteArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendMountArray(posConn POSGRPCConnection, req *pb.MountArrayRequest) (*pb.MountArrayResponse, error) {
	conn, err := dialToCliServer(posConn)
	if err != nil {
		log.Print(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()
    duration := mountArrayCmdTimeout
    if posConn.TimeoutChanged {
        duration = posConn.ReqTimeout
    }

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(duration))
	defer cancel()

	res, err := c.MountArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUnmountArray(posConn POSGRPCConnection, req *pb.UnmountArrayRequest) (*pb.UnmountArrayResponse, error) {
	conn, err := dialToCliServer(posConn)
	if err != nil {
		log.Print(err)
		errToReturn := errors.New(dialErrorMsg)
		return nil, errToReturn
	}
	defer conn.Close()
    duration := unmountArrayCmdTimeout
    if posConn.TimeoutChanged {
        duration = posConn.ReqTimeout
    }

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(duration))
	defer cancel()

	res, err := c.UnmountArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendArrayInfo(posConn POSGRPCConnection, req *pb.ArrayInfoRequest) (*pb.ArrayInfoResponse, error) {
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

	res, err := c.ArrayInfo(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendRebuildArray(posConn POSGRPCConnection, req *pb.RebuildArrayRequest) (*pb.RebuildArrayResponse, error) {
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

	res, err := c.RebuildArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListArray(posConn POSGRPCConnection, req *pb.ListArrayRequest) (*pb.ListArrayResponse, error) {
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

	res, err := c.ListArray(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetLogPreference(posConn POSGRPCConnection, req *pb.SetLogPreferenceRequest) (*pb.SetLogPreferenceResponse, error) {
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

	res, err := c.SetLogPreference(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetLogLevel(posConn POSGRPCConnection, req *pb.SetLogLevelRequest) (*pb.SetLogLevelResponse, error) {
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

	res, err := c.SetLogLevel(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendLoggerInfo(posConn POSGRPCConnection, req *pb.LoggerInfoRequest) (*pb.LoggerInfoResponse, error) {
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

	res, err := c.LoggerInfo(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateVolume(posConn POSGRPCConnection, req *pb.CreateVolumeRequest) (*pb.CreateVolumeResponse, error) {
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

	res, err := c.CreateVolume(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetLogLevel(posConn POSGRPCConnection, req *pb.GetLogLevelRequest) (*pb.GetLogLevelResponse, error) {
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

	res, err := c.GetLogLevel(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendApplyLogFilter(posConn POSGRPCConnection, req *pb.ApplyLogFilterRequest) (*pb.ApplyLogFilterResponse, error) {
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

	res, err := c.ApplyLogFilter(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateDevice(posConn POSGRPCConnection, req *pb.CreateDeviceRequest) (*pb.CreateDeviceResponse, error) {
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

	res, err := c.CreateDevice(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendScanDevice(posConn POSGRPCConnection, req *pb.ScanDeviceRequest) (*pb.ScanDeviceResponse, error) {
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

	res, err := c.ScanDevice(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListDevice(posConn POSGRPCConnection, req *pb.ListDeviceRequest) (*pb.ListDeviceResponse, error) {
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

	res, err := c.ListDevice(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetSmartLog(posConn POSGRPCConnection, req *pb.GetSmartLogRequest) (*pb.GetSmartLogResponse, error) {
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

	res, err := c.GetSmartLog(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateSubsystem(posConn POSGRPCConnection, req *pb.CreateSubsystemRequest) (*pb.CreateSubsystemResponse, error) {
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

	res, err := c.CreateSubsystem(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteSubsystem(posConn POSGRPCConnection, req *pb.DeleteSubsystemRequest) (*pb.DeleteSubsystemResponse, error) {
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

	res, err := c.DeleteSubsystem(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAddListener(posConn POSGRPCConnection, req *pb.AddListenerRequest) (*pb.AddListenerResponse, error) {
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

	res, err := c.AddListener(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListSubsystem(posConn POSGRPCConnection, req *pb.ListSubsystemRequest) (*pb.ListSubsystemResponse, error) {
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

	res, err := c.ListSubsystem(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSubsystemInfo(posConn POSGRPCConnection, req *pb.SubsystemInfoRequest) (*pb.SubsystemInfoResponse, error) {
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

	res, err := c.SubsystemInfo(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateTransport(posConn POSGRPCConnection, req *pb.CreateTransportRequest) (*pb.CreateTransportResponse, error) {
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

	res, err := c.CreateTransport(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeProperty(posConn POSGRPCConnection, req *pb.SetVolumePropertyRequest) (*pb.SetVolumePropertyResponse, error) {
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

	res, err := c.SetVolumeProperty(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendDeleteVolume(posConn POSGRPCConnection, req *pb.DeleteVolumeRequest) (*pb.DeleteVolumeResponse, error) {
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

	res, err := c.DeleteVolume(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendMountVolume(posConn POSGRPCConnection, req *pb.MountVolumeRequest) (*pb.MountVolumeResponse, error) {
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

	res, err := c.MountVolume(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUnmountVolume(posConn POSGRPCConnection, req *pb.UnmountVolumeRequest) (*pb.UnmountVolumeResponse, error) {
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

	res, err := c.UnmountVolume(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListVolume(posConn POSGRPCConnection, req *pb.ListVolumeRequest) (*pb.ListVolumeResponse, error) {
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

	res, err := c.ListVolume(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeInfo(posConn POSGRPCConnection, req *pb.VolumeInfoRequest) (*pb.VolumeInfoResponse, error) {
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

	res, err := c.VolumeInfo(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendVolumeRename(posConn POSGRPCConnection, req *pb.VolumeRenameRequest) (*pb.VolumeRenameResponse, error) {
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

	res, err := c.VolumeRename(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateQoSVolumePolicy(posConn POSGRPCConnection, req *pb.QosCreateVolumePolicyRequest) (*pb.QosCreateVolumePolicyResponse, error) {
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

	res, err := c.QosCreateVolumePolicy(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetQoSVolumePolicy(posConn POSGRPCConnection, req *pb.QosResetVolumePolicyRequest) (*pb.QosResetVolumePolicyResponse, error) {
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

	res, err := c.QosResetVolumePolicy(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListQoSPolicy(posConn POSGRPCConnection, req *pb.ListQOSPolicyRequest) (*pb.ListQOSPolicyResponse, error) {
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

	res, err := c.ListQOSPolicy(ctx, req)
	if err != nil {
		log.Print("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendListWBT(posConn POSGRPCConnection, req *pb.ListWBTRequest) (*pb.ListWBTResponse, error) {
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

    res, err := c.ListWBT(ctx, req)
    if err != nil {
        log.Print("error: ", err.Error())
        return nil, err
    }

    return res, err
}

func SendWBT(posConn POSGRPCConnection, req *pb.WBTRequest) (*pb.WBTResponse, error) {
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

    res, err := c.WBT(ctx, req)
    if err != nil {
        log.Print("error: ", err.Error())
        return nil, err
    }

    return res, err
}

