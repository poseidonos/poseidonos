
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
const uint32_t TIMELAG_SIZE{100};

struct Data
{
    virtual ~Data(void)
    {
    }
    uint32_t access{0};
};

struct AccData
{
    virtual ~AccData(void)
    {
    }
    uint32_t need_erase{0};
};

struct PerformanceData : public Data
{
    uint64_t bandwidth{0};
    uint32_t iops{0};
    std::map<uint32_t, uint32_t> packet_cnt;
};

struct AccPerformanceData : public AccData
{
    uint64_t bandwidth_avg{0};
    uint32_t iops_avg{0};
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

struct LatencyData : public Data
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

struct AccLatencyData : public AccData
{
    uint32_t mean{0};
    uint32_t min{0};
    uint32_t max{0};
    uint32_t median{0};
    uint32_t lower_quartile{0};
    uint32_t upper_quartile{0};
    uint32_t sample_count{0};

    uint32_t total_mean{0};
    uint32_t total_min{0};
    uint32_t total_max{0};
    uint32_t total_median{0};
    uint32_t total_lower_quartile{0};
    uint32_t total_upper_quartile{0};
    uint64_t total_sample_count{0};
    uint32_t overflow_warning{0};

    uint64_t timelag[TIMELAG_SIZE]{
        0,
    };
};

struct QueueData : public Data
{
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
    uint32_t total_num_req{0};
    double depth_total_avg{0.0};
};

struct UtilizationData : public Data
{
    uint64_t usage{0};
};

struct AccUtilizationData : public AccData
{
    uint64_t total_usage{0};
};

struct CountData : public Data
{
    uint64_t count_positive{0};
    uint64_t count_negative{0};
    uint64_t num_req_positive{0};
    uint64_t num_req_negative{0};
    uint64_t count{0};
    uint32_t negative{0};
};

struct AccCountData : public AccData
{
    uint64_t total_count{0};
    uint64_t total_num_req_positive{0};
    uint64_t total_num_req_negative{0};
    uint32_t negative{0};
};

} // namespace lib

#endif // AIR_DATA_H
