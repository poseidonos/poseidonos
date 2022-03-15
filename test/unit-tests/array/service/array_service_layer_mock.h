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
    MOCK_METHOD(int, Register, (string array, unsigned int arrayIndex, ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker), (override));
    MOCK_METHOD(void, Unregister, (string array, unsigned int arrayIndex), (override));
    MOCK_METHOD(IIOTranslator*, GetTranslator, (), (override));
    MOCK_METHOD(IIORecover*, GetRecover, (), (override));
    MOCK_METHOD(IIODeviceChecker*, GetDeviceChecker, (), (override));
};

} // namespace pos
