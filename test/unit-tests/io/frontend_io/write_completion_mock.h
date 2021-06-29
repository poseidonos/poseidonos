#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/write_completion.h"

namespace pos
{
class MockWriteCompletion : public WriteCompletion
{
public:
    using WriteCompletion::WriteCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
