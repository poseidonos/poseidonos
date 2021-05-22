
#ifndef AIR_QUEUE_PROCESSOR_H
#define AIR_QUEUE_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/lib/StringView.h"
#include "src/process/processor/Processor.h"

namespace process
{
class QueueProcessor : public Processor
{
private:
    void _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;

public:
    virtual ~QueueProcessor(void)
    {
    }

private:
    void _JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
        air::string_view& node_name_view, uint32_t tid, const char* tname,
        uint64_t hash_value, uint32_t filter_index) override;
};

} // namespace process

#endif // AIR_QUEUE_PROCESSOR_H
