
#include "src/collection/Collector.cpp"
#include "src/collection/Collector.h"
#include "writer_test.h"

class CollectorTest : public ::testing::Test
{
public:
    collection::PerformanceCollector* performance_collector{nullptr};
    collection::LatencyCollector* latency_collector{nullptr};
    collection::QueueCollector* queue_collector{nullptr};
    collection::CountCollector* count_collector{nullptr};
    collection::UtilizationCollector* util_collector{nullptr};

protected:
    CollectorTest()
    {
        performance_collector = new collection::PerformanceCollector{new collection::PerformanceWriter};
        latency_collector = new collection::LatencyCollector{new collection::LatencyWriter};
        queue_collector = new collection::QueueCollector{new collection::QueueWriter};
        count_collector = new collection::CountCollector{new collection::CountWriter};
        util_collector = new collection::UtilizationCollector{new collection::UtilizationWriter};
    }
    ~CollectorTest()
    {
        if (nullptr != performance_collector)
        {
            delete performance_collector;
            performance_collector = nullptr;
        }
        if (nullptr != latency_collector)
        {
            delete latency_collector;
            latency_collector = nullptr;
        }
        if (nullptr != queue_collector)
        {
            delete queue_collector;
            queue_collector = nullptr;
        }
        if (nullptr != count_collector)
        {
            delete count_collector;
            count_collector = nullptr;
        }
        if (nullptr != util_collector)
        {
            delete util_collector;
            util_collector = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
