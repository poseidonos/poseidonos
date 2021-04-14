#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/log_handler.h"

namespace pos
{
class MockLogHandlerInterface : public LogHandlerInterface
{
public:
    using LogHandlerInterface::LogHandlerInterface;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

class MockBlockWriteDoneLogHandler : public BlockWriteDoneLogHandler
{
public:
    using BlockWriteDoneLogHandler::BlockWriteDoneLogHandler;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

class MockStripeMapUpdatedLogHandler : public StripeMapUpdatedLogHandler
{
public:
    using StripeMapUpdatedLogHandler::StripeMapUpdatedLogHandler;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

class MockVolumeDeletedLogEntry : public VolumeDeletedLogEntry
{
public:
    using VolumeDeletedLogEntry::VolumeDeletedLogEntry;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

} // namespace pos
