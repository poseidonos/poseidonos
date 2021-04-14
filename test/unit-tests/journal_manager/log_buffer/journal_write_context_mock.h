#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/journal_write_context.h"

namespace pos
{
class MockLogWriteContext : public LogWriteContext
{
public:
    using LogWriteContext::LogWriteContext;
    MOCK_METHOD(uint32_t, GetLogSize, (), (override));
    MOCK_METHOD(int, GetLogGroupId, (), (override));
    MOCK_METHOD(void, SetAllocated, (int groupId, uint32_t seqNum, MetaIoCbPtr cb), (override));
    MOCK_METHOD(void, SetIoRequest, (MetaFsIoOpcode op, int fileDescriptor, uint64_t offset), (override));
    MOCK_METHOD(void, LogWriteDone, (), (override));
};

class MockMapUpdateLogWriteContext : public MapUpdateLogWriteContext
{
public:
    using MapUpdateLogWriteContext::MapUpdateLogWriteContext;
    MOCK_METHOD(void, LogWriteDone, (), (override));
};

class MockBlockMapUpdatedLogWriteContext : public BlockMapUpdatedLogWriteContext
{
public:
    using BlockMapUpdatedLogWriteContext::BlockMapUpdatedLogWriteContext;
};

class MockStripeMapUpdatedLogWriteContext : public StripeMapUpdatedLogWriteContext
{
public:
    using StripeMapUpdatedLogWriteContext::StripeMapUpdatedLogWriteContext;
};

class MockVolumeDeletedLogWriteContext : public VolumeDeletedLogWriteContext
{
public:
    using VolumeDeletedLogWriteContext::VolumeDeletedLogWriteContext;
    MOCK_METHOD(void, LogWriteDone, (), (override));
};

class MockJournalResetContext : public JournalResetContext
{
public:
    using JournalResetContext::JournalResetContext;
};

} // namespace pos
