#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/rebuild_io/rebuild_read.h"

namespace pos
{
class MockRebuildRead : public RebuildRead
{
public:
    using RebuildRead::RebuildRead;
};

} // namespace pos
