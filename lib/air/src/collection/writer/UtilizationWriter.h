
#ifndef AIR_COLLECTION_UTILIZATION_WRITER_H
#define AIR_COLLECTION_UTILIZATION_WRITER_H

#include "src/collection/writer/Writer.h"
#include "src/lib/Data.h"

namespace collection
{
class UtilizationWriter : public Writer
{
public:
    UtilizationWriter(void)
    {
    }
    virtual ~UtilizationWriter(void)
    {
    }
    inline void
    LogData(lib::Data* data, uint64_t usage) override
    {
        lib::UtilizationData* util_data = static_cast<lib::UtilizationData*>(data);
        util_data->access = true;

        util_data->usage += usage;
    }
    int
    SetSamplingRate(uint32_t rate) override
    {
        return 0;
    }
};

} // namespace collection

#endif // AIR_COLLECTION_UTILIZATION_WRITER_H
