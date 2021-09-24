#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/i_journal_volume_event_handler.h"

namespace pos
{
class MockIJournalVolumeEventHandler : public IJournalVolumeEventHandler
{
public:
    using IJournalVolumeEventHandler::IJournalVolumeEventHandler;
    MOCK_METHOD(int, WriteVolumeDeletedLog, (int volId), (override));
    MOCK_METHOD(int, TriggerMetadataFlush, (), (override));
};

} // namespace pos
