#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/msc_req.h"

namespace pos
{
class MockMetaFsControlReqMsg : public MetaFsControlReqMsg
{
public:
    using MetaFsControlReqMsg::MetaFsControlReqMsg;
};

} // namespace pos
