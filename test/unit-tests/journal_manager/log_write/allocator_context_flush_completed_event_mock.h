#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/allocator_context_flush_completed_event.h"

namespace pos
{
class MockIAllocatorContextFlushed : public IAllocatorContextFlushed
{
public:
    using IAllocatorContextFlushed::IAllocatorContextFlushed;
    MOCK_METHOD(void, AllocatorContextFlushed, (), (override));
};

class MockAllocatorContextFlushCompleted : public AllocatorContextFlushCompleted
{
public:
    using AllocatorContextFlushCompleted::AllocatorContextFlushCompleted;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
