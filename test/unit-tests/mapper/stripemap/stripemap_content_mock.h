#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/stripemap/stripemap_content.h"

namespace pos
{
class MockStripeMapContent : public StripeMapContent
{
public:
    using StripeMapContent::StripeMapContent;
    MOCK_METHOD(int, InMemoryInit, (uint64_t entrySize, uint64_t mpageSize), (override));
    MOCK_METHOD(MpageList, GetDirtyPages, (uint64_t start, uint64_t numEntries), (override));
    MOCK_METHOD(StripeAddr, GetEntry, (StripeId vsid), (override));
    MOCK_METHOD(int, SetEntry, (StripeId vsid, StripeAddr entry), (override));

    MOCK_METHOD(int, Init, (uint64_t numEntries, uint64_t entrySize, uint64_t mpageSize), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, Load, (AsyncLoadCallBack & cb), (override));
    MOCK_METHOD(int, OpenMapFile, (), (override));
    MOCK_METHOD(int, DeleteMapFile, (), (override));
    MOCK_METHOD(bool, DoesFileExist, (), (override));
    MOCK_METHOD(int, FlushDirtyPagesGiven, (MpageList dirtyPages, EventSmartPtr callback), (override));
    MOCK_METHOD(int, FlushTouchedPages, (EventSmartPtr callback), (override));
    MOCK_METHOD(int, FlushHeader, (EventSmartPtr callback), (override));
    MOCK_METHOD(int, Dump, (std::string fileName), (override));
    MOCK_METHOD(int, DumpLoad, (std::string fileName), (override));
    MOCK_METHOD(uint64_t, GetEntriesPerPage, (), (override));
};

} // namespace pos
