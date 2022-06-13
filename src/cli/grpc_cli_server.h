#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <grpc/support/log.h>

#include "proto/generated/cpp/cli.grpc.pb.h"
#include "proto/generated/cpp/cli.pb.h"

#include "src/cli/command_processor.h"
#include "src/include/grpc_server_socket_address.h"
#include "src/logger/logger.h"

#define GRPC_TIMEOUT_MESSAGE "Deadline exceeded or client cancelled."

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc_cli::PosCli;

using namespace google::protobuf;

void RunGrpcServer();
void _LogCliRequest(const google::protobuf::Message* request);
void _LogCliResponse(const google::protobuf::Message* reply, const grpc::Status status);
void _LogGrpcTimeout(const google::protobuf::Message* request, const google::protobuf::Message* reply);