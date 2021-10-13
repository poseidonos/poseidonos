#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/internal_manager.h"

namespace pos
{
class MockQosInternalManager : public QosInternalManager
{
public:
    using QosInternalManager::QosInternalManager;
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(QosInternalManagerType, GetNextManagerType, (), (override));
};

} // namespace pos
