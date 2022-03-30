#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/interface/i_array_service_consumer.h"

namespace pos
{
class MockIArrayServiceConsumer : public IArrayServiceConsumer
{
public:
    using IArrayServiceConsumer::IArrayServiceConsumer;
    MOCK_METHOD(IIOTranslator*, GetTranslator, (), (override));
    MOCK_METHOD(IIORecover*, GetRecover, (), (override));
    MOCK_METHOD(IIODeviceChecker*, GetDeviceChecker, (), (override));
    MOCK_METHOD(IIOLocker*, GetIOLocker, (PartitionType), (override));
};

} // namespace pos
