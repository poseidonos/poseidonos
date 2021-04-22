
#ifndef AIR_COLLECTION_QUEUE_WRITER_H
#define AIR_COLLECTION_QUEUE_WRITER_H

#include <cmath>
#include <random>

#include "src/collection/writer/Writer.h"
#include "src/lib/Data.h"

namespace collection
{
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
    LogData(lib::Data* data, uint64_t q_depth, uint64_t q_size) override
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
    int
    SetSamplingRate(uint32_t rate) override
    {
        if (rate >= MIN_RATIO && rate <= MAX_RATIO)
        {
            sampling_rate = rate;
            _UpdateRand();

            return 0;
        }
        else
        {
            return -2; // boundray error
        }
    }
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

#endif // AIR_COLLECTION_QUEUE_WRITER_H
