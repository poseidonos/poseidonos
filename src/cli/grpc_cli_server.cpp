#include <iostream>
#include <memory>
#include <string>

#include "src/cli/grpc_cli_server.h"

#define MAX_NUM_CONCURRENT_CLIENTS 1

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc_cli::PosCli;

CommandProcessor* pc;

class PosCliServiceImpl final : public PosCli::Service {
  // System Commands
  grpc::Status
  SystemInfo(ServerContext* context, const SystemInfoRequest* request,
                  SystemInfoResponse* reply) override
  {
      POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

      grpc::Status status = pc->ExecuteSystemInfoCommand(request, reply);

      POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());
    
      return status;
  }

  grpc::Status
  SystemStop(ServerContext* context, const SystemStopRequest* request,
                  SystemStopResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteSystemStopCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());
    
    return status;
  }

  grpc::Status
  GetSystemProperty(ServerContext* context, const GetSystemPropertyRequest* request,
                  GetSystemPropertyResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteGetSystemPropertyCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  SetSystemProperty(ServerContext* context, const SetSystemPropertyRequest* request,
                  SetSystemPropertyResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteSetSystemPropertyCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  // Telemetry Commands
  grpc::Status
  StartTelemetry(ServerContext* context, const StartTelemetryRequest* request,
                  StartTelemetryResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteStartTelemetryCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  StopTelemetry(ServerContext* context, const StopTelemetryRequest* request,
                  StopTelemetryResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteStopTelemetryCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  // Devel Commands
  grpc::Status
  ResetEventWrr(ServerContext* context, const ResetEventWrrRequest* request,
                  ResetEventWrrResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteResetEventWrrCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  ResetMbr(ServerContext* context, const ResetMbrRequest* request,
                  ResetMbrResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteResetMbrCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  StopRebuilding(ServerContext* context, const StopRebuildingRequest* request,
                  StopRebuildingResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteStopRebuildingCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  UpdateEventWrr(ServerContext* context, const UpdateEventWrrRequest* request,
                  UpdateEventWrrResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteUpdateEventWrrCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  // Array Commands
  grpc::Status
  AddSpare(ServerContext* context, const AddSpareRequest* request,
                  AddSpareResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteAddSpareCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  AutocreateArray(ServerContext* context, const AutocreateArrayRequest* request,
                  AutocreateArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteAutocreateArrayCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  CreateArray(ServerContext* context, const CreateArrayRequest* request,
                  CreateArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteCreateArrayCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  DeleteArray(ServerContext* context, const DeleteArrayRequest* request,
                  DeleteArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteDeleteArrayCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  ListArray(ServerContext* context, const ListArrayRequest* request,
                  ListArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteListArrayCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  ArrayInfo(ServerContext* context, const ArrayInfoRequest* request,
                  ArrayInfoResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteArrayInfoCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  MountArray(ServerContext* context, const MountArrayRequest* request,
                  MountArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteMountArrayCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  RemoveSpare(ServerContext* context, const RemoveSpareRequest* request,
                  RemoveSpareResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteRemoveSpareCommand(request, reply);
    
    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

    return status;
  }

  grpc::Status
  UnmountArray(ServerContext* context, const UnmountArrayRequest* request,
                  UnmountArrayResponse* reply) override
  {
    POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

    grpc::Status status = pc->ExecuteUnmountArrayCommand(request, reply);

    POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());
    
    return status;
  }
};

void RunGrpcServer() {
  pc = new CommandProcessor();

  std::string server_address(GRPC_SERVER_ADDRESS);
  PosCliServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  grpc::ResourceQuota rq;
  
  rq.SetMaxThreads(MAX_NUM_CONCURRENT_CLIENTS + 1);
  builder.SetResourceQuota(rq);
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}