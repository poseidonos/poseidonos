#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_content.h"

namespace pos
{
class MockMapContent : public MapContent
{
public:
    using MapContent::MapContent;
    MOCK_METHOD(MpageList, GetDirtyPages, (uint64_t start, uint64_t numEntries), (override));
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
