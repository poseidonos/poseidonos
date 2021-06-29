#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/flush_read_completion.h"

namespace pos
{
class MockFlushReadCompletion : public FlushReadCompletion
{
public:
    using FlushReadCompletion::FlushReadCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
