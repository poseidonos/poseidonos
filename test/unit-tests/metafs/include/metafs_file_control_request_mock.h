#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs_control_request.h"

namespace pos
{
class MockMetaFsFileControlRequest : public MetaFsFileControlRequest
{
public:
    using MetaFsFileControlRequest::MetaFsFileControlRequest;
    MOCK_METHOD(bool, IsValid, ());
};

} // namespace pos
