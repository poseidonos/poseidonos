#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/bio/ubio.h"

namespace pos
{
class MockUbio : public Ubio
{
public:
    using Ubio::Ubio;
    MOCK_METHOD(void, WaitDone, (), (override));
    MOCK_METHOD(void*, GetWholeBuffer, (), (const override));
    MOCK_METHOD(uint32_t, GetOriginCore, (), (override));
    MOCK_METHOD(void, SetSyncMode, (), (override));
    MOCK_METHOD(bool, IsSyncMode, (), (override));
    MOCK_METHOD(IArrayDevice*, GetArrayDev, (), (override));
    MOCK_METHOD(void, SetError, (IOErrorType inputErrorType), (override));
    MOCK_METHOD(bool, NeedRecovery, (), (override));
    MOCK_METHOD(bool, IsRetry, (), (override));
    MOCK_METHOD(UbioSmartPtr, GetOriginUbio, (), (override));
    MOCK_METHOD(CallbackSmartPtr, GetCallback, (), (override));
    MOCK_METHOD(void, ClearCallback, (), (override));
    MOCK_METHOD(void, ClearOrigin, (), (override));
    MOCK_METHOD(void, Complete, (IOErrorType error), (override));
    MOCK_METHOD(bool, CheckRecoveryAllowed, (), (override));
    MOCK_METHOD(UBlockDevice*, GetUBlock, (), (override));
};

} // namespace pos
