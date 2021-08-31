#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/flush_completion.h"

namespace pos
{
class MockFlushCompletion : public FlushCompletion
{
public:
    using FlushCompletion::FlushCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
