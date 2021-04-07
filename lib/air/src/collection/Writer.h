
#ifndef AIR_COLLECTION_WRITER_H
#define AIR_COLLECTION_WRITER_H

#include <time.h>

#include <cmath>
#include <random>

#include "src/lib/CType.h"
#include "src/lib/Casting.h"
#include "src/lib/Data.h"
#include "src/lib/Type.h"

namespace collection
{
class Writer
{
public:
    virtual ~Writer(void)
    {
    }
    virtual void LogData(lib::Data* data, uint32_t value1, uint64_t value2) = 0;
    virtual void InformInit(lib::AccData* data);
    virtual int SetSamplingRate(uint32_t rate) = 0;
};

class PerformanceWriter : public Writer
{
public:
    virtual ~PerformanceWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint32_t io_type, uint64_t io_size)
    {
        lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);

        perf_data->access = true;

        switch (io_type)
        {
            case (AIR_READ):
                perf_data->iops_read++;
                perf_data->bandwidth_read += io_size;
                break;
            case (AIR_WRITE):
                perf_data->iops_write++;
                perf_data->bandwidth_write += io_size;
                break;
            default:
                return;
        }

        auto entry = perf_data->packet_cnt.find(io_size);
        if (entry != perf_data->packet_cnt.end())
        {
            entry->second++;
        }
        else
        {
            if (lib::MAX_PACKET_CNT_SIZE > perf_data->packet_cnt.size())
            {
                perf_data->packet_cnt.insert({io_size, 1});
            }
        }
    }
    int SetSamplingRate(uint32_t rate);
};

class LatencyWriter : public Writer
{
public:
    virtual ~LatencyWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint32_t seq_index, uint64_t key)
    {
        lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);

        lat_data->access = true;

        if (lib::SID_SIZE <= seq_index)
        {
            return;
        }

        lib::LatencySeqData* sl_data =
            static_cast<lib::LatencySeqData*>(&lat_data->seq_data[seq_index]);

        if (lib::TimeLogState::RUN == sl_data->start_state)
        {
            lib::TimeLog time_log;
            time_log.key = key;
            clock_gettime(CLOCK_MONOTONIC, &time_log.timestamp);

            sl_data->start_v.push_back(time_log);
            sl_data->start_token--;
            if (0 >= sl_data->start_token)
            {
                sl_data->start_state = lib::TimeLogState::FULL;
            }
        }

        if (lib::TimeLogState::RUN == sl_data->end_state)
        {
            lib::TimeLog time_log;
            time_log.key = key;
            clock_gettime(CLOCK_MONOTONIC, &time_log.timestamp);

            sl_data->end_v.push_back(time_log);
            sl_data->end_token--;
            if (0 >= sl_data->end_token)
            {
                sl_data->end_state = lib::TimeLogState::FULL;
            }
        }
    }

    inline void
    AddTimelag(lib::AccLatencySeqData* lat_data, uint64_t time_lag)
    {
        if (nullptr == lat_data)
        {
            return;
        }
        lib::AccLatencySeqDataBucket* lat_bucket;
        // use current bucket?? or next new bucket??
        if ((lat_data->bucket.size() != 0) &&
            (lat_data->bucket.back()->time_lag_size < lib::TIME_LAG_SIZE))
        {
            lat_bucket = lat_data->bucket.back();
        }
        else
        {
            lat_bucket = new lib::AccLatencySeqDataBucket;
            lat_data->bucket.push_back(lat_bucket);
            lat_data->bucket_count++;
        }
        // calculate acc lat seq min/max
        if (time_lag > lat_data->max)
        {
            lat_data->max = time_lag;
        }
        if ((time_lag != 0) &&
            ((lat_data->min == 0) || (time_lag < lat_data->min)))
        {
            lat_data->min = time_lag;
        }
        // add time lag
        lat_bucket->time_lag[lat_bucket->time_lag_size] = time_lag;
        lat_bucket->time_lag_size++;
        lat_data->sample_count++;
    }

    int SetSamplingRate(uint32_t rate);

private:
};

class QueueWriter : public Writer
{
public:
    QueueWriter(void)
    {
        _UpdateRand();
    }
    virtual ~QueueWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint32_t q_depth, uint64_t q_size)
    {
        lib::QueueData* queue_data = static_cast<lib::QueueData*>(data);
        if (false == IsSampling(queue_data))
        {
            return;
        }
        queue_data->access = true;
        queue_data->sum_depth += q_depth;
        queue_data->q_size = q_size;
        queue_data->num_req++;
        if (queue_data->depth_period_max < q_depth)
        {
            queue_data->depth_period_max = q_depth;
        }
    }
    int SetSamplingRate(uint32_t rate);
    inline bool
    IsSampling(lib::QueueData* queue_data)
    {
        bool result{false};
        if (sampling_rate != queue_data->sampling_rate)
        {
            queue_data->sampling_rate = sampling_rate;
            queue_data->logging_point = 0;
            queue_data->num_called = 0;
            queue_data->mersenne = mersenne;
        }
        if (queue_data->logging_point == queue_data->num_called)
        {
            result = true;
        }
        else
        {
            result = false;
        }
        if (queue_data->num_called % sampling_rate == 0)
        {
            queue_data->num_called = 0;
            if (sampling_rate < 10)
            {
                queue_data->logging_point = sampling_rate;
            }
            else
            {
                uint32_t rand_uint = std::uniform_int_distribution<uint32_t>{
                    0, sampling_rate}(queue_data->mersenne);
                float rand_float = rand_uint * 0.8;
                queue_data->logging_point = rand_float + (sampling_rate * 0.1);
            }
        }
        queue_data->num_called++;
        return result;
    }
    uint32_t
    GetLoggingPoint(lib::QueueData* queue_data)
    {
        return queue_data->logging_point;
    }

private:
    void _UpdateRand(void);
    static const uint32_t PADDING{10};
    static const uint32_t DEFAULT_RATIO{1000}; // tmp. 1000
    static const uint32_t MIN_RATIO{1};
    static const uint32_t MAX_RATIO{10000};

    uint32_t sampling_rate{DEFAULT_RATIO};
    std::random_device rand;
    std::mt19937 mersenne;
};

} // namespace collection

#endif // AIR_COLLECTION_WRITER_H
