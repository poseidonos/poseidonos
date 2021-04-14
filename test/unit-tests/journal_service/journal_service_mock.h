#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/journal_service.h"

namespace pos
{
class MockJournalService : public JournalService
{
public:
    using JournalService::JournalService;
    MOCK_METHOD(bool, IsEnabled, (std::string arrayName), (override));
    MOCK_METHOD(void, Register, (std::string arrayName, IJournalWriter* writer), (override));
    MOCK_METHOD(void, Unregister, (std::string arrayName), (override));
    MOCK_METHOD(IJournalWriter*, GetWriter, (std::string arrayName), (override));
};

} // namespace pos
