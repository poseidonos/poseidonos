#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_perf_test.h"

namespace pos
{
class MockUtMetaFsPerf : public UtMetaFsPerf
{
public:
    using UtMetaFsPerf::UtMetaFsPerf;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

class MockTestClient : public TestClient
{
public:
    using TestClient::TestClient;
};

} // namespace pos
