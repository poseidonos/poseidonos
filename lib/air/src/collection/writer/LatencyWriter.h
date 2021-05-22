
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
    LogData(lib::Data* data, uint64_t key) override
    {
        lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);

        lat_data->access = true;

        if (lib::TimeLogState::RUN == lat_data->start_state)
        {
            lib::TimeLog time_log;
            time_log.key = key;
            clock_gettime(CLOCK_MONOTONIC, &time_log.timestamp);

            lat_data->start_v.push_back(time_log);
            lat_data->start_token--;
            if (0 >= lat_data->start_token)
            {
                lat_data->start_state = lib::TimeLogState::FULL;
            }
        }

        if (lib::TimeLogState::RUN == lat_data->end_state)
        {
            lib::TimeLog time_log;
            time_log.key = key;
            clock_gettime(CLOCK_MONOTONIC, &time_log.timestamp);

            lat_data->end_v.push_back(time_log);
            lat_data->end_token--;
            if (0 >= lat_data->end_token)
            {
                lat_data->end_state = lib::TimeLogState::FULL;
            }
        }
    }

    inline void
    AddTimelag(lib::AccLatencyData* lat_data, uint64_t timelag)
    {
        if (nullptr == lat_data)
        {
            return;
        }

        if (timelag > lat_data->max)
        {
            lat_data->max = timelag;
        }

        if ((timelag != 0) && ((lat_data->min == 0) || (timelag < lat_data->min)))
        {
            lat_data->min = timelag;
        }

        if (lib::TIMELAG_SIZE > lat_data->sample_count)
        {
            lat_data->timelag[lat_data->sample_count] = timelag;
            lat_data->sample_count++;
        }
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
