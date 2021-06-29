
#ifndef AIR_SWITCH_GEAR_H
#define AIR_SWITCH_GEAR_H

#include "src/config/ConfigInterface.h"
#include "src/data_structure/NodeManager.h"
#include "src/lib/Data.h"
#include "src/meta/NodeMeta.h"

namespace collection
{
class SwitchGear
{
public:
    SwitchGear(meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager)
    : node_meta_getter(new_node_meta_getter),
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
    node::NodeManager* node_manager{nullptr};
    const uint32_t MAX_NID_SIZE{
        cfg::GetSentenceCount(config::ParagraphType::NODE)};
};

} // namespace collection

#endif // AIR_SWITCH_GEAR_H
