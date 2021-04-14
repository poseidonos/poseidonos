#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/block_map_update_request.h"

namespace pos
{
class MockBlockMapUpdateRequest : public BlockMapUpdateRequest
{
public:
    using BlockMapUpdateRequest::BlockMapUpdateRequest;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
    MOCK_METHOD(void, _UpdateMeta, (), (override));
    MOCK_METHOD(void, _UpdateReverseMap, (Stripe & stripe), (override));
};

} // namespace pos
