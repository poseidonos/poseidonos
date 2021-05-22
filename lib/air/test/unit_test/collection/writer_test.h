
#include "src/collection/writer/CountWriter.cpp"
#include "src/collection/writer/CountWriter.h"
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.cpp"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.cpp"
#include "src/collection/writer/QueueWriter.h"
#include "src/collection/writer/UtilizationWriter.cpp"
#include "src/collection/writer/UtilizationWriter.h"
#include "src/collection/writer/Writer.cpp"
#include "src/collection/writer/Writer.h"
#include "src/lib/Casting.h"
#include "src/lib/Data.h"
#include "src/lib/Type.h"

class WriterTest : public ::testing::Test
{
public:
    collection::PerformanceWriter* performance_writer{nullptr};
    collection::LatencyWriter* latency_writer{nullptr};
    collection::QueueWriter* queue_writer{nullptr};
    collection::CountWriter* count_writer{nullptr};
    collection::UtilizationWriter* util_writer{nullptr};

protected:
    WriterTest()
    {
        performance_writer = new collection::PerformanceWriter{};
        latency_writer = new collection::LatencyWriter{};
        queue_writer = new collection::QueueWriter{};
        count_writer = new collection::CountWriter{};
        util_writer = new collection::UtilizationWriter{};
    }
    ~WriterTest()
    {
        if (nullptr != performance_writer)
        {
            delete performance_writer;
            performance_writer = nullptr;
        }
        if (nullptr != latency_writer)
        {
            delete latency_writer;
            latency_writer = nullptr;
        }
        if (nullptr != queue_writer)
        {
            delete queue_writer;
            queue_writer = nullptr;
        }
        if (nullptr != count_writer)
        {
            delete count_writer;
            count_writer = nullptr;
        }
        if (nullptr != util_writer)
        {
            delete util_writer;
            util_writer = nullptr;
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
