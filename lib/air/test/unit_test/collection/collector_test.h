
#include "src/collection/Collector.h"
#include "src/collection/Collector.cpp"
#include "writer_test.h"

class CollectorTest : public ::testing::Test
{
public:
    collection::PerformanceCollector* performance_collector {nullptr};
    collection::LatencyCollector* latency_collector {nullptr};
    collection::QueueCollector* queue_collector {nullptr};
protected:
    CollectorTest () {
        performance_collector = new collection::PerformanceCollector
        {new collection::PerformanceWriter};
        latency_collector = new collection::LatencyCollector
        {new collection::LatencyWriter};
        queue_collector = new collection::QueueCollector
        {new collection::QueueWriter};
    }
    ~CollectorTest () {
        if (nullptr != performance_collector) {
            delete performance_collector;
            performance_collector = nullptr;
        }
        if (nullptr != latency_collector) {
            delete latency_collector;
            latency_collector = nullptr;
        }
        if (nullptr != queue_collector) {
            delete queue_collector;
            queue_collector = nullptr;
        }
    }
    void SetUp() override {}
    void TearDown() override {}
};
