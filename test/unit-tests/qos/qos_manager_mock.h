#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_manager.h"

namespace pos
{
class MockQosManager : public QosManager
{
public:
    using QosManager::QosManager;
    MOCK_METHOD(int64_t, GetEventWeightWRR, (BackendEvent event), (override));
    MOCK_METHOD(bool, IsFeQosEnabled, (), (override));
    MOCK_METHOD(void, _Finalize, (), (override));
};

} // namespace pos
