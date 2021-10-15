#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/stripemap/stripemap_manager.h"

namespace pos
{
class MockStripeMapManager : public StripeMapManager
{
public:
    using StripeMapManager::StripeMapManager;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, LoadStripeMapFile, (), (override));
    MOCK_METHOD(int, FlushDirtyPagesGiven, (MpageList list, EventSmartPtr cb), (override));
    MOCK_METHOD(int, FlushTouchedPages, (EventSmartPtr cb), (override));
    MOCK_METHOD(void, MapFlushDone, (int mapId), (override));
    MOCK_METHOD(void, WaitAllPendingIoDone, (), (override));
    MOCK_METHOD(void, WaitWritePendingIoDone, (), (override));
    MOCK_METHOD(StripeAddr, GetLSA, (StripeId vsid), (override));
    MOCK_METHOD(LsidRefResult, GetLSAandReferLsid, (StripeId vsid), (override));
    MOCK_METHOD(StripeId, GetRandomLsid, (StripeId vsid), (override));
    MOCK_METHOD(int, SetLSA, (StripeId vsid, StripeId lsid, StripeLoc loc), (override));
    MOCK_METHOD(bool, IsInUserDataArea, (StripeAddr entry), (override));
    MOCK_METHOD(bool, IsInWriteBufferArea, (StripeAddr entry), (override));
    MOCK_METHOD(MpageList, GetDirtyStripeMapPages, (int vsid), (override));
    MOCK_METHOD(StripeMapContent*, GetStripeMapContent, (), (override));
    MOCK_METHOD(void, SetStripeMapContent, (StripeMapContent * content), (override));
    MOCK_METHOD(int, Dump, (std::string fileName), (override));
    MOCK_METHOD(int, DumpLoad, (std::string fileName), (override));
};

} // namespace pos
