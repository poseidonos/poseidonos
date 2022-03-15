#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/interface/i_array_service_producer.h"

namespace pos
{
class MockIArrayServiceProducer : public IArrayServiceProducer
{
public:
    using IArrayServiceProducer::IArrayServiceProducer;
    MOCK_METHOD(int, Register, (string array, unsigned int arrayIndex, ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker), (override));
    MOCK_METHOD(void, Unregister, (string array, unsigned int arrayIndex), (override));
};

} // namespace pos
