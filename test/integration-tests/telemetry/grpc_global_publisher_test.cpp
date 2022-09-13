#include <gtest/gtest.h>
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"


using namespace pos;

TEST(GrpcGlobalPublisher, GrpcGlobalPublisher_TestPublishCounterMetric)
{
    // Given: a grpc client
    string publisherName = "test_publisher";
    TelemetryPublisher publisher(publisherName);
    TelemetryClientSingleton::Instance()->RegisterPublisher(&publisher);
    POSMetric metric("test_counter_metric_name", MT_COUNT);
    metric.SetCountValue(1234);
    metric.AddLabel("array_unique_id", "11223344");
    metric.AddLabel("volume_unique_id", "22334455");

    // When 1: publish()
    int ret = publisher.PublishMetric(metric);

    // Then -1: Server is not running.
    EXPECT_EQ(-1, ret); // TODO: Activate after MetricManager applied to TelemetryManager Server
}
