#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/flush_metadata.h"

namespace pos
{
class MockFlushMetadata : public FlushMetadata
{
public:
    using FlushMetadata::FlushMetadata;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
