#include <gtest/gtest.h>
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"


namespace pos
{
TEST(GrpcGlobalPublisher, GrpcGlobalPublisher_TestPublishServer)
{
    // Given: a grpc client
    GrpcGlobalPublisher* grpcClient = new GrpcGlobalPublisher(nullptr);
    POSMetricVector* v = new POSMetricVector();
    POSMetric metric("n", MT_COUNT);
    v->push_back(metric);

    // When 1: publish()
    int ret = grpcClient->PublishToServer(nullptr, v);

    // Then -1: Server is not running
    EXPECT_EQ(-1, ret); // TODO: Activate after MetricManager applied to TelemetryManager Server
    delete v;
    delete grpcClient;
}

TEST(GrpcGlobalPublisher, GrpcGlobalPublisher_TestPublishServerWithStringMetric)
{

    // Given: a grpc client
    GrpcGlobalPublisher* grpcClient = new GrpcGlobalPublisher(nullptr);
    POSMetricVector* v = new POSMetricVector();
    POSMetric metric("n", MT_COUNT);
    v->push_back(metric);

    // When 1: publish()
    int ret = grpcClient->PublishToServer(nullptr, v);

    // Then -1: Server is not running
    EXPECT_EQ(-1, ret); // TODO: Activate after MetricManager applied to TelemetryManager Server
    delete grpcClient;
}

} // namespace pos
