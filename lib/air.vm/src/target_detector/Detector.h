
#ifndef AIR_DETECTOR_H
#define AIR_DETECTOR_H

#include "src/data_structure/NodeManager.h"
#include "src/lib/Design.h"

namespace detect
{
class Detector
{
public:
    explicit Detector(node::NodeManager* new_node_manager)
    : node_manager(new_node_manager)
    {
    }
    ~Detector(void)
    {
    }
    void Process(void);

private:
    node::NodeManager* node_manager{nullptr};
    void _CreateNodeDataArray(uint32_t tid);
    void _DeleteNodeDataArray(void);
};

} // namespace detect

#endif // AIR_DETECTOR_H
