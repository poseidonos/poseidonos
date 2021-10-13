#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/correction_manager.h"

namespace pos
{
class MockQosCorrectionManager : public QosCorrectionManager
{
public:
    using QosCorrectionManager::QosCorrectionManager;
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(QosInternalManagerType, GetNextManagerType, (), (override));
};

} // namespace pos
