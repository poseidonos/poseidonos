#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/interface/pbr_updater_factory.h"

namespace pbr
{
class MockPbrUpdaterFactory : public PbrUpdaterFactory
{
public:
    using PbrUpdaterFactory::PbrUpdaterFactory;
    MOCK_METHOD(IPbrUpdater*, GetPbrUpdater, (vector<pos::UblockSharedPtr> devs), (override));
    MOCK_METHOD(IPbrUpdater*, GetPbrUpdater, (vector<string> fileList), (override));
};

} // namespace pbr
