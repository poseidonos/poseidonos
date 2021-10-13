#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/processing_manager.h"

namespace pos
{
class MockQosProcessingManager : public QosProcessingManager
{
public:
    using QosProcessingManager::QosProcessingManager;
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(QosInternalManagerType, GetNextManagerType, (), (override));
};

} // namespace pos
