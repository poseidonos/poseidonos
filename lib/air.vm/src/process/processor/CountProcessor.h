
#ifndef AIR_COUNT_PROCESSOR_H
#define AIR_COUNT_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/process/processor/Processor.h"

namespace process
{
class CountProcessor : public Processor
{
public:
    virtual ~CountProcessor(void)
    {
    }

private:
    bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
};

} // namespace process

#endif // AIR_COUNT_PROCESSOR_H
