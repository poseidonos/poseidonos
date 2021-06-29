#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/testdouble/stub/metafs_util_api_stub.h"

namespace pos
{
class MockMfsUtilApi : public MfsUtilApi
{
public:
    using MfsUtilApi::MfsUtilApi;
    MOCK_METHOD(size_t, GetFileSize, (int fd), (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize, (int fd), (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize, (MetaFilePropertySet & prop), (override));
    MOCK_METHOD(size_t, GetTheBiggestExtentSize, (MetaFilePropertySet & prop), (override));
};

} // namespace pos
