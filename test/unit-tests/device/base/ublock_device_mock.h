#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/base/ublock_device.h"

namespace pos
{
class MockDeviceProperty : public DeviceProperty
{
public:
    using DeviceProperty::DeviceProperty;
};

class MockUBlockDevice : public UBlockDevice
{
public:
    using UBlockDevice::UBlockDevice;
    MOCK_METHOD(bool, Open, (), (override));
    MOCK_METHOD(uint32_t, Close, (), (override));
    MOCK_METHOD(int, SubmitAsyncIO, (UbioSmartPtr bio), (override));
    MOCK_METHOD(int, CompleteIOs, (), (override));
    MOCK_METHOD(const char*, GetName, (), (override));
    MOCK_METHOD(uint64_t, GetSize, (), (override));
    MOCK_METHOD(DeviceType, GetType, (), (override));
    MOCK_METHOD(std::string, GetSN, (), (const, override));
    MOCK_METHOD(std::string, GetMN, (), (override));
    MOCK_METHOD(DeviceClass, GetClass, (), (override));
    MOCK_METHOD(int, GetNuma, (), (override));
    MOCK_METHOD(DeviceProperty, GetProperty, (), (override));
    MOCK_METHOD(void, SetClass, (DeviceClass cls), (override));
    MOCK_METHOD(void, AddPendingErrorCount, (uint32_t errorsToAdd), (override));
    MOCK_METHOD(void, SubtractPendingErrorCount, (uint32_t errorsToSubtract), (override));
    MOCK_METHOD(void, SetDedicatedIOWorker, (IOWorker * ioWorker), (override));
    MOCK_METHOD(IOWorker*, GetDedicatedIOWorker, (), (override));
    MOCK_METHOD(DeviceContext*, _AllocateDeviceContext, (), (override));
    MOCK_METHOD(void, _ReleaseDeviceContext, (DeviceContext * deviceContextToRelease), (override));
    MOCK_METHOD(bool, WrapupOpenDeviceSpecific, (), (override));
    MOCK_METHOD(void*, GetByteAddress, (), (override));
};

} // namespace pos
