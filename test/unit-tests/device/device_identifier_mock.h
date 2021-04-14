#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/device_identifier.h"

namespace pos
{
class MockDeviceIdentifierPredicate : public DeviceIdentifierPredicate
{
public:
    using DeviceIdentifierPredicate::DeviceIdentifierPredicate;
    MOCK_METHOD(bool, operator,(UblockSharedPtr dev), (const, override));
};

class MockDevNamePred : public DevNamePred
{
public:
    using DevNamePred::DevNamePred;
    MOCK_METHOD(bool, operator,(UblockSharedPtr dev), (const, override));
};

class MockDevSNPred : public DevSNPred
{
public:
    using DevSNPred::DevSNPred;
    MOCK_METHOD(bool, operator,(UblockSharedPtr dev), (const, override));
};

class MockDeviceIdentifier : public DeviceIdentifier
{
public:
    using DeviceIdentifier::DeviceIdentifier;
    MOCK_METHOD(Predicate, GetPredicate, (), (override));
};

class MockDevName : public DevName
{
public:
    using DevName::DevName;
};

class MockDevUid : public DevUid
{
public:
    using DevUid::DevUid;
};

} // namespace pos
