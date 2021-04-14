
#ifndef AIR_DETECTOR_H
#define AIR_DETECTOR_H

#include "src/lib/Design.h"
#include "src/profile_data/node/NodeManager.h"

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
    int _CreateThread(uint32_t tid);
    int _DeleteThread(void);
};

} // namespace detect

#endif // AIR_DETECTOR_H
