#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/load/pbr_loader.h"

namespace pbr
{
class MockPbrLoader : public PbrLoader
{
public:
    using PbrLoader::PbrLoader;
    MOCK_METHOD(int, Load, (vector<AteData*>& out), (override));
};

} // namespace pbr
