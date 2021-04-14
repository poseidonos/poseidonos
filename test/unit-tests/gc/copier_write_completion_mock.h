#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/copier_write_completion.h"

namespace pos
{
class MockGcFlushCompletion : public GcFlushCompletion
{
public:
    using GcFlushCompletion::GcFlushCompletion;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
