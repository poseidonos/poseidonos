#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_service/journal_service.h"

namespace pos
{
class MockJournalService : public JournalService
{
public:
    using JournalService::JournalService;
    MOCK_METHOD(bool, IsEnabled, (std::string arrayName), (override));
    MOCK_METHOD(bool, IsEnabled, (int arrayId), (override));
    MOCK_METHOD(void, Register, (std::string arrayName, int arrayId, IJournalManager* journal, IJournalWriter* writer, IJournalStatusProvider* provider), (override));
    MOCK_METHOD(void, Unregister, (std::string arrayName), (override));
    MOCK_METHOD(IJournalWriter*, GetWriter, (std::string arrayName), (override));
    MOCK_METHOD(IJournalWriter*, GetWriter, (int arrayId), (override));
};

} // namespace pos
