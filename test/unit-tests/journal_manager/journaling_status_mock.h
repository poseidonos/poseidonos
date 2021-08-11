#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/journaling_status.h"

namespace pos
{
class MockJournalingStatus : public JournalingStatus
{
public:
    using JournalingStatus::JournalingStatus;
    MOCK_METHOD(JournalManagerStatus, Get, (), (override));
    MOCK_METHOD(void, Set, (JournalManagerStatus to), (override));
};

} // namespace pos
