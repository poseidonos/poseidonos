#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/journal_status_provider_stub.h"

namespace pos
{
class MockJournalStatusProviderStub : public JournalStatusProviderStub
{
public:
    using JournalStatusProviderStub::JournalStatusProviderStub;
    MOCK_METHOD(JsonElement, GetJournalStatus, (), (override));
};

} // namespace pos
