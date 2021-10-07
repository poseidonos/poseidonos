#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/i_journal_manager.h"

namespace pos
{
class MockIJournalManager : public IJournalManager
{
public:
    using IJournalManager::IJournalManager;
    MOCK_METHOD(bool, IsEnabled, (), (override));
};

} // namespace pos
