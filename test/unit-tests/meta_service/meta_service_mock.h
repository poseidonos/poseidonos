#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_service/meta_service.h"

namespace pos
{
class MockMetaService : public MetaService
{
public:
    using MetaService::MetaService;
    MOCK_METHOD(void, Register, (std::string arrayName, int arrayId, IMetaUpdater* mapUpdater), (override));
    MOCK_METHOD(void, Unregister, (std::string arrayName), (override));
    MOCK_METHOD(IMetaUpdater*, GetMetaUpdater, (std::string arrayName), (override));
    MOCK_METHOD(IMetaUpdater*, GetMetaUpdater, (int arrayId), (override));
};

} // namespace pos
