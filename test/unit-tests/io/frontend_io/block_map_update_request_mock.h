#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/frontend_io/block_map_update_request.h"

namespace pos
{
class MockBlockMapUpdateRequest : public BlockMapUpdateRequest
{
public:
    using BlockMapUpdateRequest::BlockMapUpdateRequest;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
    MOCK_METHOD(bool, _UpdateMeta, (), (override));
};

} // namespace pos
