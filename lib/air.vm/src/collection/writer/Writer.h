
#ifndef AIR_COLLECTION_WRITER_H
#define AIR_COLLECTION_WRITER_H

#include "src/lib/Data.h"

namespace collection
{
class Writer
{
public:
    virtual ~Writer(void)
    {
    }
    virtual void LogData(lib::Data* data, uint64_t value1, uint64_t value2) = 0;
    virtual void InformInit(lib::AccData* data);
    virtual int SetSamplingRate(uint32_t rate) = 0;
};

} // namespace collection

#endif // AIR_COLLECTION_WRITER_H
