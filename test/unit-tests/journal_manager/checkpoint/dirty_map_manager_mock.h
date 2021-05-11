#include <gmock/gmock.h>

#include <string>
#include <list>
#include <vector>

#include "src/journal_manager/checkpoint/dirty_map_manager.h"

namespace pos
{
class MockDirtyMapManager : public DirtyMapManager
{
public:
    using DirtyMapManager::DirtyMapManager;
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration), (override));
    MOCK_METHOD(void, LogFilled, (int logGroupId, MapPageList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
};

} // namespace pos
