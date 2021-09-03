#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_map_update_completion.h"

namespace pos
{
class MockGcMapUpdateCompletion : public GcMapUpdateCompletion
{
public:
    using GcMapUpdateCompletion::GcMapUpdateCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
