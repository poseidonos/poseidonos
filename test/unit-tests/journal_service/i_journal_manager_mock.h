#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/i_journal_manager.h"

namespace pos
{
class MockIJournalManager : public IJournalManager
{
public:
    using IJournalManager::IJournalManager;
    MOCK_METHOD(bool, IsEnabled, (), (override));
};

} // namespace pos
