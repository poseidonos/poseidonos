#include "telemetry_server.h"

#include <grpcpp/grpcpp.h>

#include "proto/generated/cpp/telemetry.grpc.pb.h"

using namespace pos;

void
TelemetryServer::Run(void)
{
    grpc::ServerBuilder builder;
    TelemetryManager::Service tmService;
    builder.AddListeningPort(SERVER_ADDRESS, grpc::InsecureServerCredentials());
    builder.RegisterService(&tmService);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();

}
