
#ifndef AIR_COLLECTION_PERFORMANCE_WRITER_H
#define AIR_COLLECTION_PERFORMANCE_WRITER_H

#include "src/collection/writer/Writer.h"
#include "src/lib/Data.h"

namespace collection
{
class PerformanceWriter : public Writer
{
public:
    virtual ~PerformanceWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint64_t io_size) override
    {
        lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
        perf_data->access = true;

        perf_data->iops++;
        perf_data->bandwidth += io_size;

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
    int
    SetSamplingRate(uint32_t rate) override
    {
        return 0;
    }
};

} // namespace collection

#endif // AIR_COLLECTION_PERFORMANCE_WRITER_H
