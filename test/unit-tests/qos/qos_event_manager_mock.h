#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_event_manager.h"

namespace pos
{
class MockQosEventManager : public QosEventManager
{
public:
    using QosEventManager::QosEventManager;
};

} // namespace pos
