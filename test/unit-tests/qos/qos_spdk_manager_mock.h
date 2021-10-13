#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_spdk_manager.h"

namespace pos
{
class MockQosSpdkManager : public QosSpdkManager
{
public:
    using QosSpdkManager::QosSpdkManager;
};

} // namespace pos
