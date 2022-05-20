// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: cli.proto

#include "cli.pb.h"
#include "cli.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace grpc_cli {

static const char* PosCli_method_names[] = {
  "/grpc_cli.PosCli/SystemInfo",
  "/grpc_cli.PosCli/SystemStop",
  "/grpc_cli.PosCli/GetSystemProperty",
};

std::unique_ptr< PosCli::Stub> PosCli::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< PosCli::Stub> stub(new PosCli::Stub(channel, options));
  return stub;
}

PosCli::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_SystemInfo_(PosCli_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SystemStop_(PosCli_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetSystemProperty_(PosCli_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status PosCli::Stub::SystemInfo(::grpc::ClientContext* context, const ::grpc_cli::SystemInfoRequest& request, ::grpc_cli::SystemInfoResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::grpc_cli::SystemInfoRequest, ::grpc_cli::SystemInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SystemInfo_, context, request, response);
}

void PosCli::Stub::experimental_async::SystemInfo(::grpc::ClientContext* context, const ::grpc_cli::SystemInfoRequest* request, ::grpc_cli::SystemInfoResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::grpc_cli::SystemInfoRequest, ::grpc_cli::SystemInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SystemInfo_, context, request, response, std::move(f));
}

void PosCli::Stub::experimental_async::SystemInfo(::grpc::ClientContext* context, const ::grpc_cli::SystemInfoRequest* request, ::grpc_cli::SystemInfoResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SystemInfo_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::SystemInfoResponse>* PosCli::Stub::PrepareAsyncSystemInfoRaw(::grpc::ClientContext* context, const ::grpc_cli::SystemInfoRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::grpc_cli::SystemInfoResponse, ::grpc_cli::SystemInfoRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SystemInfo_, context, request);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::SystemInfoResponse>* PosCli::Stub::AsyncSystemInfoRaw(::grpc::ClientContext* context, const ::grpc_cli::SystemInfoRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSystemInfoRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status PosCli::Stub::SystemStop(::grpc::ClientContext* context, const ::grpc_cli::SystemStopRequest& request, ::grpc_cli::SystemStopResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::grpc_cli::SystemStopRequest, ::grpc_cli::SystemStopResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SystemStop_, context, request, response);
}

void PosCli::Stub::experimental_async::SystemStop(::grpc::ClientContext* context, const ::grpc_cli::SystemStopRequest* request, ::grpc_cli::SystemStopResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::grpc_cli::SystemStopRequest, ::grpc_cli::SystemStopResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SystemStop_, context, request, response, std::move(f));
}

void PosCli::Stub::experimental_async::SystemStop(::grpc::ClientContext* context, const ::grpc_cli::SystemStopRequest* request, ::grpc_cli::SystemStopResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SystemStop_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::SystemStopResponse>* PosCli::Stub::PrepareAsyncSystemStopRaw(::grpc::ClientContext* context, const ::grpc_cli::SystemStopRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::grpc_cli::SystemStopResponse, ::grpc_cli::SystemStopRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SystemStop_, context, request);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::SystemStopResponse>* PosCli::Stub::AsyncSystemStopRaw(::grpc::ClientContext* context, const ::grpc_cli::SystemStopRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSystemStopRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status PosCli::Stub::GetSystemProperty(::grpc::ClientContext* context, const ::grpc_cli::GetSystemPropertyRequest& request, ::grpc_cli::GetSystemPropertyResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::grpc_cli::GetSystemPropertyRequest, ::grpc_cli::GetSystemPropertyResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetSystemProperty_, context, request, response);
}

void PosCli::Stub::experimental_async::GetSystemProperty(::grpc::ClientContext* context, const ::grpc_cli::GetSystemPropertyRequest* request, ::grpc_cli::GetSystemPropertyResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::grpc_cli::GetSystemPropertyRequest, ::grpc_cli::GetSystemPropertyResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetSystemProperty_, context, request, response, std::move(f));
}

void PosCli::Stub::experimental_async::GetSystemProperty(::grpc::ClientContext* context, const ::grpc_cli::GetSystemPropertyRequest* request, ::grpc_cli::GetSystemPropertyResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetSystemProperty_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::GetSystemPropertyResponse>* PosCli::Stub::PrepareAsyncGetSystemPropertyRaw(::grpc::ClientContext* context, const ::grpc_cli::GetSystemPropertyRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::grpc_cli::GetSystemPropertyResponse, ::grpc_cli::GetSystemPropertyRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetSystemProperty_, context, request);
}

::grpc::ClientAsyncResponseReader< ::grpc_cli::GetSystemPropertyResponse>* PosCli::Stub::AsyncGetSystemPropertyRaw(::grpc::ClientContext* context, const ::grpc_cli::GetSystemPropertyRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetSystemPropertyRaw(context, request, cq);
  result->StartCall();
  return result;
}

PosCli::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PosCli_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PosCli::Service, ::grpc_cli::SystemInfoRequest, ::grpc_cli::SystemInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](PosCli::Service* service,
             ::grpc::ServerContext* ctx,
             const ::grpc_cli::SystemInfoRequest* req,
             ::grpc_cli::SystemInfoResponse* resp) {
               return service->SystemInfo(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PosCli_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PosCli::Service, ::grpc_cli::SystemStopRequest, ::grpc_cli::SystemStopResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](PosCli::Service* service,
             ::grpc::ServerContext* ctx,
             const ::grpc_cli::SystemStopRequest* req,
             ::grpc_cli::SystemStopResponse* resp) {
               return service->SystemStop(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PosCli_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PosCli::Service, ::grpc_cli::GetSystemPropertyRequest, ::grpc_cli::GetSystemPropertyResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](PosCli::Service* service,
             ::grpc::ServerContext* ctx,
             const ::grpc_cli::GetSystemPropertyRequest* req,
             ::grpc_cli::GetSystemPropertyResponse* resp) {
               return service->GetSystemProperty(ctx, req, resp);
             }, this)));
}

PosCli::Service::~Service() {
}

::grpc::Status PosCli::Service::SystemInfo(::grpc::ServerContext* context, const ::grpc_cli::SystemInfoRequest* request, ::grpc_cli::SystemInfoResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status PosCli::Service::SystemStop(::grpc::ServerContext* context, const ::grpc_cli::SystemStopRequest* request, ::grpc_cli::SystemStopResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status PosCli::Service::GetSystemProperty(::grpc::ServerContext* context, const ::grpc_cli::GetSystemPropertyRequest* request, ::grpc_cli::GetSystemPropertyResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace grpc_cli

