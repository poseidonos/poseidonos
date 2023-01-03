#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/update/pbr_updater.h"

namespace pbr
{
class MockPbrUpdater : public PbrUpdater
{
public:
    using PbrUpdater::PbrUpdater;
    MOCK_METHOD(int, Update, (AteData* ateData), (override));
    MOCK_METHOD(int, Clear, (), (override));
};

} // namespace pbr
