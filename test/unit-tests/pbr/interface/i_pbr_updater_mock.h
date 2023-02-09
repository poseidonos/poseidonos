#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/interface/i_pbr_updater.h"

namespace pbr
{
class MockIPbrUpdater : public IPbrUpdater
{
public:
    using IPbrUpdater::IPbrUpdater;
    MOCK_METHOD(int, Update, (AteData* ateData), (override));
    MOCK_METHOD(int, Clear, (), (override));
};

} // namespace pbr
