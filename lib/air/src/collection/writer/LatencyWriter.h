
#ifndef AIR_COLLECTION_LATENCY_WRITER_H
#define AIR_COLLECTION_LATENCY_WRITER_H

#include <time.h>

#include "src/collection/writer/Writer.h"
#include "src/lib/Data.h"

namespace collection
{
class LatencyWriter : public Writer
{
public:
    virtual ~LatencyWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint64_t seq_index, uint64_t key) override
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

    int
    SetSamplingRate(uint32_t rate) override
    {
        return 0;
    }

private:
};

} // namespace collection

#endif // AIR_COLLECTION_LATENCY_WRITER_H
