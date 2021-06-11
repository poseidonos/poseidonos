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
    MOCK_METHOD(void, Init, (LogWriteContextFactory* logFactory, LogGroupReleaser* releaser,
        LogWriteHandler* logWritter, JournalConfiguration* journalConfiguration,
        IContextManager* contextManager), (override));
    MOCK_METHOD(int, VolumeDeleted, (int volID), (override));
    MOCK_METHOD(void, MetaFlushed, (), (override));
    MOCK_METHOD(void, VolumeDeletedLogWriteDone, (int volumeId), (override));
};

} // namespace pos
