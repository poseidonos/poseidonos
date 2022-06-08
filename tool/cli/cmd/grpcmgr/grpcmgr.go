package grpcmgr

import (
	"cli/cmd/globals"
	"context"
	"pnconnector/src/log"
	"time"

	pb "cli/api"

	"google.golang.org/grpc"
)

const timeout = 1

func SendSystemInfoRpc(req *pb.SystemInfoRequest) (*pb.SystemInfoResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.SystemInfo(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSystemStopRpc(req *pb.SystemStopRequest) (*pb.SystemStopResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.SystemStop(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendGetSystemPropertyRpc(req *pb.GetSystemPropertyRequest) (*pb.GetSystemPropertyResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.GetSystemProperty(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendSetSystemPropertyRpc(req *pb.SetSystemPropertyRequest) (*pb.SetSystemPropertyResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.SetSystemProperty(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStartTelemetryRpc(req *pb.StartTelemetryRequest) (*pb.StartTelemetryResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.StartTelemetry(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopTelemetryRpc(req *pb.StopTelemetryRequest) (*pb.StopTelemetryResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.StopTelemetry(ctx, req)

	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetEventWrrPolicyRpc(req *pb.ResetEventWrrRequest) (*pb.ResetEventWrrResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.ResetEventWrr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendResetMbrRpc(req *pb.ResetMbrRequest) (*pb.ResetMbrResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.ResetMbr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendStopRebuildingRpc(req *pb.StopRebuildingRequest) (*pb.StopRebuildingResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.StopRebuilding(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendUpdatEventWrr(req *pb.UpdateEventWrrRequest) (*pb.UpdateEventWrrResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.UpdateEventWrr(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendAddSpare(req *pb.AddSpareRequest) (*pb.AddSpareResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.AddSpare(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}

func SendCreateArray(req *pb.CreateArrayRequest) (*pb.CreateArrayResponse, error) {
	conn, err := grpc.Dial(globals.GrpcServerAddress, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Error("cannot send a request to cli server: not connected")
		return nil, err
	}
	defer conn.Close()

	c := pb.NewPosCliClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*timeout)
	defer cancel()

	res, err := c.CreateArray(ctx, req)
	if err != nil {
		log.Error("error: ", err.Error())
		return nil, err
	}

	return res, err
}
