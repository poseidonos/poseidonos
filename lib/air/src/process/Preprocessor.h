
#ifndef AIR_PREPROCESSOR_H
#define AIR_PREPROCESSOR_H

#include "src/collection/Writer.h"
#include "src/config/ConfigInterface.h"
#include "src/lib/Data.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/profile_data/node/NodeManager.h"

namespace process
{
class Preprocessor
{
public:
    Preprocessor(meta::NodeMetaGetter* new_node_meta_getter,
        meta::GlobalMetaGetter* new_global_meta_getter,
        node::NodeManager* new_node_manager)
    : node_meta_getter(new_node_meta_getter),
      global_meta_getter(new_global_meta_getter),
      node_manager(new_node_manager)
    {
    }
    virtual ~Preprocessor(void)
    {
    }
    void Run(int option);

private:
    void _MatchKey(lib::LatencySeqData* curr_data, lib::LatencySeqData* next_data,
        lib::AccLatencySeqData* acc_data);
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    node::NodeManager* node_manager{nullptr};
    collection::LatencyWriter latency_writer{};
    static const uint32_t NANOS{1000000000};
    static const uint64_t MAX_TIME{500000000}; // 500 ms
    const uint32_t MAX_NID_SIZE{
        config::ConfigInterface::GetArrSize(config::ConfigType::NODE)};
};

} // namespace process

#endif // AIR_PREPROCESSOR_H
