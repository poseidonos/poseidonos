#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/journal_volume_event_handler.h"

namespace pos
{
class MockJournalVolumeEventHandler : public JournalVolumeEventHandler
{
public:
    using JournalVolumeEventHandler::JournalVolumeEventHandler;
    MOCK_METHOD(void, Init, (LogWriteContextFactory * logFactory, DirtyMapManager* dirtyPages, LogWriteHandler* logWritter, JournalConfiguration* journalConfiguration, IAllocatorCtx* allocatorCtx), (override));
    MOCK_METHOD(int, VolumeDeleted, (int volID), (override));
    MOCK_METHOD(void, AllocatorContextFlushed, (), (override));
};

} // namespace pos
