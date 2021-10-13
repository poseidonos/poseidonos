#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/monitoring_manager.h"

namespace pos
{
class MockQosMonitoringManager : public QosMonitoringManager
{
public:
    using QosMonitoringManager::QosMonitoringManager;
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(QosInternalManagerType, GetNextManagerType, (), (override));
};

} // namespace pos
