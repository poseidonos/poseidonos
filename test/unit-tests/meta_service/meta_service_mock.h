#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/meta_service/meta_service.h"

namespace pos
{
class MockMetaService : public MetaService
{
public:
    using MetaService::MetaService;
    MOCK_METHOD(void, Register, (std::string arrayName, int arrayId, IMetaUpdater* mapUpdater, IJournalStatusProvider* journalStatusProvider), (override));
    MOCK_METHOD(void, Unregister, (std::string arrayName), (override));
    MOCK_METHOD(IMetaUpdater*, GetMetaUpdater, (std::string arrayName), (override));
    MOCK_METHOD(IMetaUpdater*, GetMetaUpdater, (int arrayId), (override));
    MOCK_METHOD(IJournalStatusProvider*, GetJournalStatusProvider, (std::string arrayName), (override));
    MOCK_METHOD(IJournalStatusProvider*, GetJournalStatusProvider, (int arrayId), (override));
};

} // namespace pos
