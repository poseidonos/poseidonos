#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/testdouble/stub/metafs_mgmt_api_stub.h"

namespace pos
{
class MockMetaFsFileControlApi : public MetaFsFileControlApi
{
public:
    using MetaFsFileControlApi::MetaFsFileControlApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Create, (std::string & fileName, uint64_t fileByteSize, MetaFilePropertySet prop), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Open, (std::string & fileName), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Close, (uint32_t fd), (override));
};

} // namespace pos
