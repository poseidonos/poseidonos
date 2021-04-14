#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_data_manager.h"

namespace pos
{
class MockQosDataManager : public QosDataManager
{
public:
    using QosDataManager::QosDataManager;
};

} // namespace pos
