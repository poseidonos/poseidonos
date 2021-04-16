#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/status/journal_status_provider.h"

namespace pos
{
class MockJournalStatusProvider : public JournalStatusProvider
{
public:
    using JournalStatusProvider::JournalStatusProvider;
    MOCK_METHOD(void, Init, (ILogBufferStatus * bufferStatusProvider, JournalConfiguration* journalConfig, ICheckpointStatus* checkpointStatusProvider), (override));
    MOCK_METHOD(ElementList, GetJournalStatus, (), (override));
};

} // namespace pos
