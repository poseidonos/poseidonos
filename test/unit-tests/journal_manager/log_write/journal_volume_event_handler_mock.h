#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/journal_volume_event_handler.h"

namespace pos
{
class MockJournalVolumeEventHandler : public JournalVolumeEventHandler
{
public:
    using JournalVolumeEventHandler::JournalVolumeEventHandler;
    MOCK_METHOD(void, Init, (LogWriteContextFactory* logFactory, CheckpointManager* cpManager, DirtyMapManager* dirtyManager, LogWriteHandler* logWritter, JournalConfiguration* journalConfiguration, IContextManager* contextManager, EventScheduler* scheduler), (override));
    MOCK_METHOD(int, WriteVolumeDeletedLog, (int volId), (override));
    MOCK_METHOD(int, TriggerMetadataFlush, (), (override));
    MOCK_METHOD(void, MetaFlushed, (), (override));
    MOCK_METHOD(void, VolumeDeletedLogWriteDone, (int volumeId), (override));
};

} // namespace pos
