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
    MOCK_METHOD(void, Register, (string array, ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker), (override));
    MOCK_METHOD(void, Unregister, (string array), (override));
};

} // namespace pos
