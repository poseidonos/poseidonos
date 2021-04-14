#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs_return_code.h"

namespace pos
{
template<typename StatusCodeType, typename ReturnDataType>
class MockMetaFsReturnCode : public MetaFsReturnCode<StatusCodeType, ReturnDataType>
{
public:
    using MetaFsReturnCode::MetaFsReturnCode;
};

} // namespace pos
