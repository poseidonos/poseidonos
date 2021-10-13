#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_array_manager.h"

namespace pos
{
class MockQosArrayManager : public QosArrayManager
{
public:
    using QosArrayManager::QosArrayManager;
};

} // namespace pos
