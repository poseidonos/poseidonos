
#ifndef AIR_PERFORMANCE_PROCESSOR_H
#define AIR_PERFORMANCE_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/lib/StringView.h"
#include "src/process/processor/Processor.h"

namespace process
{
class PerformanceProcessor : public Processor
{
private:
    void _JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
        air::string_view& node_name_view, uint32_t tid, const char* tname,
        uint64_t hash_value, uint32_t filter_index) override;

public:
    virtual ~PerformanceProcessor(void)
    {
    }

private:
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
};

} // namespace process

#endif // AIR_PERFORMANCE_PROCESSOR_H
