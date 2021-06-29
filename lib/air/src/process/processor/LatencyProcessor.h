
#ifndef AIR_LATENCY_PROCESSOR_H
#define AIR_LATENCY_PROCESSOR_H

#include "src/lib/Data.h"
#include "src/lib/StringView.h"
#include "src/process/processor/Processor.h"

namespace process
{
class LatencyProcessor : public Processor
{
public:
    virtual ~LatencyProcessor(void)
    {
    }

private:
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _Calculate(lib::AccLatencyData* data);
    void _JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
        air::string_view& node_name_view, uint32_t tid, const char* tname,
        uint64_t hash_value, uint32_t filter_index) override;

    static const uint64_t OVERFLOW_THRESHOLD{0x7FFFFFFFFFFFFFFF};
};

} // namespace process

#endif // AIR_LATENCY_PROCESSOR_H
