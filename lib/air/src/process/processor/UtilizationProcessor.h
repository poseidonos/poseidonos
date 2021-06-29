
#ifndef AIR_UTILIZATION_PROCESSOR_H
#define AIR_UTILIZATION_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/lib/StringView.h"
#include "src/process/processor/Processor.h"

namespace process
{
class UtilizationProcessor : public Processor
{
private:
    void _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
        air::string_view& node_name_view, uint32_t tid, const char* tname,
        uint64_t hash_value, uint32_t filter_index) override;

public:
    virtual ~UtilizationProcessor(void)
    {
    }
};

} // namespace process

#endif // AIR_UTILIZATION_PROCESSOR_H
