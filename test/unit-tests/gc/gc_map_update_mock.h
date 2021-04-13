#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_map_update.h"

namespace pos
{
class MockGcMapUpdate : public GcMapUpdate
{
public:
    using GcMapUpdate::GcMapUpdate;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
