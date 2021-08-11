#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/rebuild/rebuild_progress.h"

namespace pos
{
class MockPartitionProgress : public PartitionProgress
{
public:
    using PartitionProgress::PartitionProgress;
};

class MockRebuildProgress : public RebuildProgress
{
public:
    using RebuildProgress::RebuildProgress;
};

} // namespace pos
