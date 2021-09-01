package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"time"

	pb "collector/proto/pos.telemetry"

	"google.golang.org/grpc"
)

const (
	defaultServerIp   = "127.0.0.1"
	defaultServerPort = "50051"
	defaultTimeoutSec = 10
)

func MakeServerAddress(ip *string, port *string) string {
	return *ip + ":" + *port
}

func main() {
	ip := flag.String("i", defaultServerIp, "grpc server ip")
	port := flag.String("p", defaultServerPort, "grpc server port")
	timeoutSec := flag.Int("t", defaultTimeoutSec, "timeout second")

	flag.Parse()

	serverAddress := MakeServerAddress(ip, port)
	timeout := time.Duration(*timeoutSec) * time.Second

	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())

	conn, err := grpc.Dial(serverAddress, opts...)
	if err != nil {
		log.Fatalf("did not connect: %v", serverAddress)
	}
	defer conn.Close()

	client := pb.NewTelemetryManagerClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()

	var request pb.CollectRequest
	response, err2 := client.Collect(ctx, &request)
	if err2 != nil {
		log.Fatalf("response error: %v", err2)
	}
	fmt.Println(response)

}
