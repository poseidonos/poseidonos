#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_request.h"

namespace pos
{
class MockMetaFsIoRequest : public MetaFsIoRequest
{
public:
    using MetaFsIoRequest::MetaFsIoRequest;
};

} // namespace pos
