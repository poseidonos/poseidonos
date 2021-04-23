
#ifndef AIR_DATA_H
#define AIR_DATA_H

#include <time.h>

#include <cstdint>
#include <map>
#include <random>
#include <vector>

namespace lib
{
const uint32_t IDLE_THRESHOLD{5};
const uint32_t MAX_PACKET_CNT_SIZE{10};

struct Data
{
    virtual ~Data(void)
    {
    }
    uint32_t access{0};
};

struct AccData
{
    uint32_t need_erase{0};
};

struct PerformanceData : public Data
{
    uint64_t bandwidth_read{0};
    uint64_t bandwidth_write{0};
    uint64_t bandwidth_total{0};
    uint32_t iops_read{0};
    uint32_t iops_write{0};
    uint32_t iops_total{0};
    std::map<uint32_t, uint32_t> packet_cnt;
};

struct AccPerformanceData : public AccData
{
    uint64_t bandwidth_read_avg{0};
    uint64_t bandwidth_write_avg{0};
    uint32_t iops_read_avg{0};
    uint32_t iops_write_avg{0};
    uint32_t time_spent{0};
};

enum class TimeLogState : uint32_t
{
    IDLE, // Processor(TimingDistributor) set
    RUN,  // SwitchGear set, "Only" this state can log
    STOP, // SwitchGear set
    FULL, // Collector set, when token empty
    DONE  // Preprocessor sed
};

struct TimeLog
{
    uint64_t key{0};
    timespec timestamp{
        0,
    };
};

const uint64_t SID_SIZE{10};
const uint32_t TIME_LAG_SIZE{30};
struct LatencySeqData
{
    std::vector<struct TimeLog> start_v;
    TimeLogState start_state{TimeLogState::IDLE};
    int32_t start_token{0};
    int32_t start_size{100}; // default
    uint32_t start_match_count{0};
    int32_t start_deadline{-45};

    std::vector<struct TimeLog> end_v;
    TimeLogState end_state{TimeLogState::IDLE};
    int32_t end_token{0};
    int32_t end_size{100}; // default
    uint32_t end_match_count{0};
    int32_t end_deadline{-45};
};

struct LatencyData : public Data
{
    LatencySeqData seq_data[SID_SIZE];
};

struct AccLatencySeqDataBucket
{
    uint32_t time_lag[TIME_LAG_SIZE]{
        0,
    };
    uint32_t time_lag_size{0};
};

struct AccLatencySeqData
{
    uint32_t mean{0};
    uint32_t min{0};
    uint32_t max{0};
    uint32_t median{0};
    uint32_t lower_quartile{0};
    uint32_t upper_quartile{0};
    uint32_t sample_count{0};
    uint32_t bucket_count{0};

    uint32_t total_mean{0};
    uint32_t total_min{0};
    uint32_t total_max{0};
    uint32_t total_median{0};
    uint32_t total_lower_quartile{0};
    uint32_t total_upper_quartile{0};
    uint64_t total_sample_count{0};
    uint32_t overflow_warning{0};

    std::vector<struct AccLatencySeqDataBucket*> bucket;
};

struct AccLatencyData : public AccData
{
    AccLatencySeqData seq_data[SID_SIZE];
};

struct QueueData : public Data
{
    uint32_t q_size{0};
    uint32_t num_req{0};
    uint64_t sum_depth{0};
    float depth_period_avg{0.0};
    uint64_t depth_period_max{0};
    uint32_t dummy_u32{0};
    uint32_t logging_point{0};
    uint32_t num_called{0};
    uint32_t sampling_rate{0};
    std::mt19937 mersenne{
        0,
    };
};

struct AccQueueData : public AccData
{
    uint32_t depth_total_max{0};
    uint32_t time_spent{0};
    double depth_total_avg{0.0};
};

const uint64_t ENUM_SIZE{3};

struct UtilizationData : public Data
{
    uint64_t usage[ENUM_SIZE]{
        0,
    };
    double percent[ENUM_SIZE]{
        0.0,
    };
    uint64_t sum{0};
};

struct AccUtilizationData : public AccData
{
    uint64_t total_usage[ENUM_SIZE]{
        0,
    };
    double total_percent[ENUM_SIZE]{
        0.0,
    };
    uint64_t total_sum{0};
};

struct CountData : public Data
{
    uint64_t count[ENUM_SIZE]{
        0,
    };
    uint64_t num_req[ENUM_SIZE]{
        0,
    };
};

struct AccCountData : public AccData
{
    uint64_t total_count[ENUM_SIZE]{
        0,
    };
    uint64_t total_num_req[ENUM_SIZE]{
        0,
    };
    double total_count_avg[ENUM_SIZE]{
        0.0,
    };
};

} // namespace lib

#endif // AIR_DATA_H
