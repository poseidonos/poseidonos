
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
    LogData(lib::Data* data, uint64_t value) override
    {
        lib::CountData* count_data = static_cast<lib::CountData*>(data);
        count_data->access = true;

        if (MAX_INT64_VALUE < value)
        {
            count_data->count_negative += ((NEGATIVE_ONE - value) + 1);
            count_data->num_req_negative += 1;
        }
        else
        {
            count_data->count_positive += value;
            count_data->num_req_positive += 1;
        }
    }
    int
    SetSamplingRate(uint32_t rate) override
    {
        return 0;
    }

private:
    const uint64_t MAX_INT64_VALUE{9223372036854775807};
    const uint64_t NEGATIVE_ONE{0xffffffffffffffff};
};

} // namespace collection

#endif // AIR_COLLECTION_COUNT_WRITER_H
