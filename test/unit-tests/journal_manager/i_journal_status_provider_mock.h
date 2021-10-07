#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/i_journal_status_provider.h"

namespace pos
{
class MockIJournalStatusProvider : public IJournalStatusProvider
{
public:
    using IJournalStatusProvider::IJournalStatusProvider;
    MOCK_METHOD(bool, IsJournalEnabled, (), (override));
    MOCK_METHOD(ElementList, GetJournalStatus, (), (override));
};

} // namespace pos
