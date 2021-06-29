#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/read_completion.h"

namespace pos
{
class MockReadCompletion : public ReadCompletion
{
public:
    using ReadCompletion::ReadCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
