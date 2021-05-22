
#ifndef AIR_PREPROCESSOR_H
#define AIR_PREPROCESSOR_H

#include "src/collection/writer/LatencyWriter.h"
#include "src/config/ConfigInterface.h"
#include "src/data_structure/NodeManager.h"
#include "src/lib/Data.h"
#include "src/meta/NodeMeta.h"

namespace process
{
class Preprocessor
{
public:
    Preprocessor(meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager)
    : node_meta_getter(new_node_meta_getter),
      node_manager(new_node_manager)
    {
    }
    virtual ~Preprocessor(void)
    {
    }
    void Run(int option);

private:
    void _MatchKey(lib::LatencyData* curr_data, lib::LatencyData* next_data,
        lib::AccLatencyData* acc_data);
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    node::NodeManager* node_manager{nullptr};
    collection::LatencyWriter latency_writer{};
    static const uint64_t SEC2NANOS{1000000000}; // 1s -> 10^9
    static const uint64_t MAX_TIME{900000000};   // 900 ms
    const uint32_t MAX_NID_SIZE{cfg::GetSentenceCount(config::ParagraphType::NODE)};
};

} // namespace process

#endif // AIR_PREPROCESSOR_H
