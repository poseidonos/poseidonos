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
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(MapList, GetDirtyList, (int logGroupId), (override));
    MOCK_METHOD(MapList, GetTotalDirtyList, (), (override));
    MOCK_METHOD(void, DeleteDirtyList, (int volumeId), (override));
    MOCK_METHOD(void, LogFilled, (int logGroupId, MapList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
};

} // namespace pos
