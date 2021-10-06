#include "proto/generated/cpp/telemetry.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "proto/generated/cpp/telemetry.pb.h"
#include "proto/generated/cpp/telemetry.grpc.pb.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/grpc_wrapper/protobuf_create_channel.h"
#include "src/telemetry/telemetry_manager/telemetry_manager_service.h"

using namespace ::grpc;

namespace pos {

// This is to demonstrate how to instantiate grpc server for telemetry.proto
// Reference: https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/greeter_server.cc
class TestTelemetryManager final : public TelemetryManager::Service {
public:
    virtual ::grpc::Status configure(
        ::grpc::ServerContext* context,
        const ConfigureMetadataRequest* request,
        ConfigureMetadataResponse* response) override
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "configure has been called");
        response->set_successful(true);
        response->set_collect_latency_ms(1);
        return ::grpc::Status::OK;
    }
    virtual ::grpc::Status publish(
        ::grpc::ServerContext* context,
        const PublishRequest* request,
        PublishResponse* response) override
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "publish has been called. This is going to be failed");
        const string errorMsg = "mock invalid arg error";
        return ::grpc::Status(StatusCode::INVALID_ARGUMENT, errorMsg);
    }
    virtual ::grpc::Status collect(
        ::grpc::ServerContext* context,
        const ::CollectRequest* request,
        ::CollectResponse* response) override
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "collect has been called");
        return ::grpc::Status::OK;
    }
    virtual ::grpc::Status enable(
        ::grpc::ServerContext* context,
        const ::EnableRequest* request,
        ::EnableResponse* response) override
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "enable has been called");
        return ::grpc::Status::OK;
    }
    virtual ::grpc::Status disable(
        ::grpc::ServerContext* context,
        const ::DisableRequest* request,
        ::DisableResponse* response) override
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "disable has been called");
        return ::grpc::Status::OK;
    }
private:
    bool _enabled = true;
};
TEST(TelemetryManager, testIfGrpcServerIsUpAndShutDownAsExpected)
{
    // Given: an address
    string server_address("0.0.0.0:50051");
    // Given: a grpc server
    TestTelemetryManager tm;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&tm);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    usleep(1000); // adding some jitter for the server to be up properly
    POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "Server listening on {}", server_address);
    // server->Wait(); // this will be a blocking call, probably for a prod code
    // Given: a grpc client
    std::shared_ptr<Channel> channel
        = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<TelemetryManager::Stub> stub_
        = TelemetryManager::NewStub(channel);
    POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "Client is ready to talk to {}", server_address);
    // When 1: configure()
    ClientContext context1;
    ConfigureMetadataRequest confReq;
    confReq.set_git_hash("some-git");
    confReq.set_host_name("some-host");
    confReq.set_host_type("some-type");
    confReq.set_ip_addr("some-ip");
    confReq.set_application_name("some-app");
    ConfigureMetadataResponse confRes;
    Status status = stub_->configure(&context1, confReq, &confRes);
    // Then 1
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(true, confRes.successful());
    ASSERT_EQ(1, confRes.collect_latency_ms());
    // When 2: publish()
    ClientContext context2;
    PublishRequest pubReq;
    PublishResponse pubRes;
    status = stub_->publish(&context2, pubReq, &pubRes);
    // Then 2
    ASSERT_FALSE(status.ok());
    ASSERT_EQ(StatusCode::INVALID_ARGUMENT, status.error_code());
    server->Shutdown();
}
TEST(TelemetryManager, testIfGrpcServerClient)
{
    string server_address("0.0.0.0:50051");
    // Given: a grpc server
    TelemetryManagerService* tmServer = new TelemetryManagerService();
    usleep(1000);
    // Given: a grpc client
    ProtoBufCreateChannel* tmClient = new ProtoBufCreateChannel(server_address);
    tmServer->StopService();
    tmServer->StartService();
    // When 1: publish()
    Status status;
    status = tmClient->PublishToServer();
    // Then 1
    ASSERT_TRUE(status.ok());
    // When 2: configure()
    status = tmClient->ConfigureToServer();
    // Then 2
    ASSERT_TRUE(status.ok());
    // When 3: collect()
    status = tmClient->CollectToServer();
    // Then 3
    ASSERT_TRUE(status.ok());
    // When 4: enable()
    status = tmClient->EnableToServer();
    // Then 4
    ASSERT_TRUE(status.ok());
    // When 5: disable()
    status = tmClient->DisableToServer();
    // Then 5
    ASSERT_TRUE(status.ok());

    delete tmClient;
    delete tmServer;
}

TEST(TelemetryManager, testIfGrpcServerClient_fail)
{
    string server_address("0.0.0.0:50051");
    // Given: a grpc server
    TelemetryManagerService* tmServer = new TelemetryManagerService();
    usleep(1000);
    // Given: a grpc client
    ProtoBufCreateChannel* tmClient = new ProtoBufCreateChannel(server_address);
    tmServer->StopService();
    // When 1: publish()
    Status status;
    status = tmClient->PublishToServer();
    // Then 1
    ASSERT_FALSE(status.ok());
    // When 2: configure()
    status = tmClient->ConfigureToServer();
    // Then 2
    ASSERT_FALSE(status.ok());
    // When 3: collect()
    status = tmClient->CollectToServer();
    // Then 3
    ASSERT_FALSE(status.ok());
    // When 4: enable()
    status = tmClient->EnableToServer();
    // Then 4
    ASSERT_FALSE(status.ok());
    // When 5: disable()
    status = tmClient->DisableToServer();
    // Then 5
    ASSERT_FALSE(status.ok());

    delete tmClient;
    delete tmServer;
}

}