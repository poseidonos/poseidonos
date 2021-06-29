#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_journal_status_wbt_command.h"

namespace pos
{
class MockGetJournalStatusWbtCommand : public GetJournalStatusWbtCommand
{
public:
    using GetJournalStatusWbtCommand::GetJournalStatusWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
