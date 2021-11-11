#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/reversemap/reversemap_manager.h"

namespace pos
{
class MockReverseMapManager : public ReverseMapManager
{
public:
    using ReverseMapManager::ReverseMapManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(ReverseMapPack*, GetReverseMapPack, (StripeId wbLsid), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (bool gcDest), (override));
    MOCK_METHOD(uint64_t, GetReverseMapPerStripeFileSize, (), (override));
    MOCK_METHOD(uint64_t, GetWholeReverseMapFileSize, (), (override));
    MOCK_METHOD(int, LoadWholeReverseMap, (char* pBuffer), (override));
    MOCK_METHOD(int, StoreWholeReverseMap, (char* pBuffer), (override));
    MOCK_METHOD(void, WaitAllPendingIoDone, (), (override));
};

} // namespace pos
