
#include "src/lib/Data.h"

class DataTest : public ::testing::Test
{
public:
    lib::PerformanceData* perf_data{nullptr};
    lib::AccLatencyData* lat_data{nullptr};
    lib::QueueData* queue_data{nullptr};

protected:
    DataTest()
    {
        perf_data = new lib::PerformanceData{};
        lat_data = new lib::AccLatencyData{};
        queue_data = new lib::QueueData{};
    }
    ~DataTest() override
    {
        if (nullptr != perf_data)
        {
            delete perf_data;
            perf_data = nullptr;
        }
        if (nullptr != lat_data)
        {
            delete lat_data;
            lat_data = nullptr;
        }
        if (nullptr != queue_data)
        {
            delete queue_data;
            queue_data = nullptr;
        }
    }
    void
    SetUp() override
    {
        perf_data->iops = 100;
        lat_data->max = 0x0000FEAD;
        queue_data->sum_depth = 0xFF00EE00DD00CA00;
        queue_data->depth_period_avg = 3.21f;
    }
    void
    TearDown() override
    {
    }
};
