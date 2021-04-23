
#ifndef AIR_UTILIZATION_PROCESSOR_H
#define AIR_UTILIZATION_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/process/processor/Processor.h"

namespace process
{
class UtilizationProcessor : public Processor
{
public:
    virtual ~UtilizationProcessor(void)
    {
    }

private:
    bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
};

} // namespace process

#endif // AIR_UTILIZATION_PROCESSOR_H
