#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <grpc/support/log.h>

#include "proto/generated/cpp/cli.grpc.pb.h"
#include "proto/generated/cpp/cli.pb.h"

#include "src/cli/command_processor.h"

#define GRPC_SERVER_ADDRESS "0.0.0.0:50055"

void RunGrpcServer();