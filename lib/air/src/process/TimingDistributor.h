
#ifndef AIR_TIMING_DISTRIBUTOR_H
#define AIR_TIMING_DISTRIBUTOR_H

#include <random>

#include "src/config/ConfigInterface.h"
#include "src/data_structure/NodeManager.h"
#include "src/meta/NodeMeta.h"

namespace process
{
class TimingDistributor
{
public:
    TimingDistributor(void)
    {
    }
    ~TimingDistributor(void)
    {
    }
    void SetTiming(meta::NodeMetaGetter* node_meta_getter,
        node::NodeManager* node_manager);

private:
    void _ResetTiming(lib::LatencyData* curr_data,
        lib::LatencyData* next_data, int32_t time_value);
    std::random_device rand_device;
    std::default_random_engine rand_engine{rand_device()};
    std::uniform_int_distribution<int32_t> dist{1, 50};
    static const int32_t MATCH_LOW_WM{5};
    static const int32_t MATCH_HIGH_WM{30};
    const uint32_t MAX_NID_SIZE{cfg::GetSentenceCount(config::ParagraphType::NODE)};
};

} // namespace process

#endif // AIR_TIMING_DISTRIBUTOR_H
