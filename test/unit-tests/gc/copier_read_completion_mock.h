#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/copier_read_completion.h"

namespace pos
{
class MockCopierReadCompletion : public CopierReadCompletion
{
public:
    using CopierReadCompletion::CopierReadCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
