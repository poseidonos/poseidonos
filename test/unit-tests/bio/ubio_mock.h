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
    MOCK_METHOD(uint32_t, GetOriginCore, (), (override));
    MOCK_METHOD(void, SetSyncMode, (), (override));
    MOCK_METHOD(bool, IsSyncMode, (), (override));
    MOCK_METHOD(IArrayDevice*, GetArrayDev, (), (override));
    MOCK_METHOD(void, SetError, (IOErrorType), (override));
    MOCK_METHOD(bool, NeedRecovery, (), (override));
};

} // namespace pos
