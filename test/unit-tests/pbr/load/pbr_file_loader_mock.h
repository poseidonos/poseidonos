#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/load/pbr_file_loader.h"

namespace pbr
{
class MockPbrFileLoader : public PbrFileLoader
{
public:
    using PbrFileLoader::PbrFileLoader;
    MOCK_METHOD(int, Load, (vector<AteData*>& out), (override));
};

} // namespace pbr
