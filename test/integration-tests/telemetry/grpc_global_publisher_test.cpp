#include <gtest/gtest.h>
#include "proto/generated/cpp/telemetry.pb.h"
#include "proto/generated/cpp/telemetry.grpc.pb.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_manager/telemetry_manager_service.h"


namespace pos
{
TEST(GrpcGlobalPublisher, GrpcGlobalPublisher_TestPublishServer)
{
    // Given: a grpc server
    string server_address("0.0.0.0:50051");
    TelemetryManagerService* tmServer = new TelemetryManagerService();
    usleep(1000);

    // Given: a grpc client
    GrpcGlobalPublisher* grpcClient = new GrpcGlobalPublisher(nullptr);
    POSMetricVector* v = new POSMetricVector();
    POSMetric metric("n", MT_COUNT);
    v->push_back(metric);

    // When 1: publish()
    int ret = grpcClient->PublishToServer(nullptr, v);

    // Then 1
    EXPECT_EQ(0, ret); // TODO: Activate after MetricManager applied to TelemetryManager Server
    delete v;
    delete grpcClient;
    delete tmServer;
}

TEST(GrpcGlobalPublisher, GrpcGlobalPublisher_TestPublishServerWithStringMetric)
{
    // Given: a grpc server
    string server_address("0.0.0.0:50051");
    TelemetryManagerService* tmServer = new TelemetryManagerService();
    usleep(1000);

    // Given: a grpc client
    GrpcGlobalPublisher* grpcClient = new GrpcGlobalPublisher(nullptr);
    POSMetricVector* v = new POSMetricVector();
    POSMetric metric("n", MT_COUNT);
    v->push_back(metric);

    // When 1: publish()
    int ret = grpcClient->PublishToServer(nullptr, v);

    // Then 1
    EXPECT_EQ(0, ret); // TODO: Activate after MetricManager applied to TelemetryManager Server
    delete grpcClient;
    delete tmServer;
}

} // namespace pos
