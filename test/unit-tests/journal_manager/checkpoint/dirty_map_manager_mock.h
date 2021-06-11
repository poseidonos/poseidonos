#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/dirty_map_manager.h"

namespace pos
{
class MockDirtyMapManager : public DirtyMapManager
{
public:
    using DirtyMapManager::DirtyMapManager;
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration), (override));
    MOCK_METHOD(MapPageList, GetDirtyList, (int logGroupId), (override));
    MOCK_METHOD(MapPageList, GetTotalDirtyList, (), (override));
    MOCK_METHOD(void, LogFilled, (int logGroupId, MapPageList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
};

} // namespace pos
