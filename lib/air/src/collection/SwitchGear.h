
#ifndef AIR_SWITCH_GEAR_H
#define AIR_SWITCH_GEAR_H

#include "src/config/ConfigInterface.h"
#include "src/lib/Data.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/profile_data/node/NodeManager.h"

namespace collection
{
class SwitchGear
{
public:
    SwitchGear(meta::NodeMetaGetter* new_node_meta_getter,
        meta::GlobalMetaGetter* new_global_meta_getter,
        node::NodeManager* new_node_manager)
    : node_meta_getter(new_node_meta_getter),
      global_meta_getter(new_global_meta_getter),
      node_manager(new_node_manager)
    {
    }
    virtual ~SwitchGear(void)
    {
    }
    void Run(void);

private:
    void _CheckDeadline(lib::Data* data);
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    node::NodeManager* node_manager{nullptr};
    const uint32_t MAX_NID_SIZE{
        config::ConfigInterface::GetArrSize(config::ConfigType::NODE)};
};

} // namespace collection

#endif // AIR_SWITCH_GEAR_H
