#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/i_journal_status_provider.h"

namespace pos
{
class MockIJournalStatusProvider : public IJournalStatusProvider
{
public:
    using IJournalStatusProvider::IJournalStatusProvider;
    MOCK_METHOD(JsonElement, GetJournalStatus, (), (override));
};

} // namespace pos
