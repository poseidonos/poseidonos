#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/buffer_offset_allocator.h"

namespace pos
{
class MockBufferOffsetAllocator : public BufferOffsetAllocator
{
public:
    using BufferOffsetAllocator::BufferOffsetAllocator;
    MOCK_METHOD(void, Init, (LogGroupReleaser * releaser, JournalConfiguration* journalConfiguration), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, AllocateBuffer, (uint32_t logSize, uint64_t& allocatedOffset), (override));
    MOCK_METHOD(void, LogWriteCanceled, (int logGroupId), (override));
    MOCK_METHOD(void, LogFilled, (int logGroupId, MapList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
    MOCK_METHOD(LogGroupStatus, GetBufferStatus, (int logGroupId), (override));
    MOCK_METHOD(uint32_t, GetSequenceNumber, (int logGroupId), (override));
    MOCK_METHOD(int, GetLogGroupId, (uint64_t fileOffset), (override));
};

} // namespace pos
