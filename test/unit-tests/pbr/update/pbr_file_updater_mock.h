#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/update/pbr_file_updater.h"

namespace pbr
{
class MockPbrFileUpdater : public PbrFileUpdater
{
public:
    using PbrFileUpdater::PbrFileUpdater;
    MOCK_METHOD(int, Update, (AteData* ateData), (override));
    MOCK_METHOD(int, Clear, (), (override));
};

} // namespace pbr
