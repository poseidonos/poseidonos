#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_internal.h"

namespace pos
{
class MockIContextInternal : public IContextInternal
{
public:
    using IContextInternal::IContextInternal;
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
};

} // namespace pos
