#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/monitoring_manager_array.h"

namespace pos
{
class MockQosMonitoringManagerArray : public QosMonitoringManagerArray
{
public:
    using QosMonitoringManagerArray::QosMonitoringManagerArray;
};

} // namespace pos
