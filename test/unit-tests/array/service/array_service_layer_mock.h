#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/array_service_layer.h"

namespace pos
{
class MockArrayServiceLayer : public ArrayServiceLayer
{
public:
    using ArrayServiceLayer::ArrayServiceLayer;
    MOCK_METHOD(void, Register, (string array, ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker), (override));
    MOCK_METHOD(void, Unregister, (string array), (override));
    MOCK_METHOD(IIOTranslator*, GetTranslator, (), (override));
    MOCK_METHOD(IIORecover*, GetRecover, (), (override));
    MOCK_METHOD(IIOLocker*, GetLocker, (), (override));
    MOCK_METHOD(IIODeviceChecker*, GetDeviceChecker, (), (override));
};

} // namespace pos
