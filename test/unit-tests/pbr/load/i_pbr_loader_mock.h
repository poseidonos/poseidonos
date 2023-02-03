#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/load/i_pbr_loader.h"

namespace pbr
{
class MockIPbrLoader : public IPbrLoader
{
public:
    using IPbrLoader::IPbrLoader;
    MOCK_METHOD(int, Load, (vector<AteData*>& out), (override));
};

} // namespace pbr
