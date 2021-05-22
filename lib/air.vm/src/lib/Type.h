
#ifndef AIR_TYPE_H
#define AIR_TYPE_H

#include <cstdint>

namespace air
{
enum class ProcessorType : uint32_t
{
    PERFORMANCE = 0,
    LATENCY,
    QUEUE,
    UTILIZATION,
    COUNT,
    PROCESSORTYPE_NULL
};

struct NodeMetaData
{
    // Mandatory
    uint32_t nid{0};
    ProcessorType processor_type{ProcessorType::PROCESSORTYPE_NULL};
    bool run{false};
    uint32_t group_id{0};
    uint32_t index_size{0};
    uint32_t filter_size{0};

    // Optional
    uint32_t sample_ratio{1000};
};

} // namespace air

#endif // AIR_TYPE_H
