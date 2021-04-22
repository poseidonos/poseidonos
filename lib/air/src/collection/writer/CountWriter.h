
#ifndef AIR_COLLECTION_COUNT_WRITER_H
#define AIR_COLLECTION_COUNT_WRITER_H

#include "src/collection/writer/Writer.h"
#include "src/lib/Data.h"

namespace collection
{
class CountWriter : public Writer
{
public:
    CountWriter(void)
    {
    }
    virtual ~CountWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint64_t enum_index, uint64_t count) override
    {
        lib::CountData* count_data = static_cast<lib::CountData*>(data);
        count_data->access = true;

        if (lib::ENUM_SIZE <= enum_index)
        {
            return;
        }

        count_data->count[enum_index] += count;
        count_data->num_req[enum_index] += 1;
    }
    int
    SetSamplingRate(uint32_t rate) override
    {
        return 0;
    }
};

} // namespace collection

#endif // AIR_COLLECTION_COUNT_WRITER_H
