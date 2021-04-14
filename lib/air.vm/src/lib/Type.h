
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

struct Node
{
    // Mandatory
    uint32_t nid{0};
    ProcessorType processor_type{ProcessorType::PROCESSORTYPE_NULL};
    bool enable{false};

    // Optional
    uint32_t sample_ratio{1000};
    uint32_t child_id[4]{
        0,
    };
    int32_t group_id{-1};
};

} // namespace air

#endif // AIR_TYPE_H
