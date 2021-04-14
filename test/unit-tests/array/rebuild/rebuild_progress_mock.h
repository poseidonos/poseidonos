#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/rebuild/rebuild_progress.h"

namespace pos
{
class MockRebuildProgress : public RebuildProgress
{
public:
    using RebuildProgress::RebuildProgress;
};

} // namespace pos
