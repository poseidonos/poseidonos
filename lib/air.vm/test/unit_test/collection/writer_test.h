
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.cpp"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.cpp"
#include "src/collection/writer/QueueWriter.h"
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

protected:
    WriterTest()
    {
        performance_writer = new collection::PerformanceWriter{};
        latency_writer = new collection::LatencyWriter{};
        queue_writer = new collection::QueueWriter{};
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
