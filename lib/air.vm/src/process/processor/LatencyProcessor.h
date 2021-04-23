
#ifndef AIR_LATENCY_PROCESSOR_H
#define AIR_LATENCY_PROCESSOR_H

#include "src/lib/Data.h"
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
    bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _InitData(lib::Data* air_data, lib::AccData* acc_data) override;
    void _Calculate(lib::AccLatencySeqData* lat_data);

    static const uint64_t OVERFLOW_THRESHOLD{0x7FFFFFFFFFFFFFFF};
};

} // namespace process

#endif // AIR_LATENCY_PROCESSOR_H
