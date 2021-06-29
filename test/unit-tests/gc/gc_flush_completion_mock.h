#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_flush_completion.h"

namespace pos
{
class MockGcFlushCompletion : public GcFlushCompletion
{
public:
    using GcFlushCompletion::GcFlushCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
