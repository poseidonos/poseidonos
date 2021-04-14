#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/internal_write_completion.h"

namespace pos
{
class MockInternalWriteCompletion : public InternalWriteCompletion
{
public:
    using InternalWriteCompletion::InternalWriteCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
