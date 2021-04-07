#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

#include "../log_buffer/journal_write_context.h"
#include "../log_write/buffer_write_done_notifier.h"

namespace ibofos
{
class LogGroupReleaser;

struct OffsetInFile // TODO(huijeong.kim) to be removed
{
    int id;
    int offset;
    uint32_t seqNum;
};

enum class LogGroupBufferStatus
{
    INVALID = -1,
    INIT = 0,
    ACTIVE,
    FULL
};

struct LogGroupStatus
{
    uint32_t seqNum;

    LogGroupBufferStatus status;
    std::atomic<bool> waitingToBeFilled;

    std::mutex countLock;
    uint32_t numLogsAdded;
    uint32_t numLogsFilled;

    uint64_t nextOffset;

    LogGroupStatus(void)
    {
        Reset();
    }

    LogGroupStatus(const LogGroupStatus& input)
    {
        seqNum = input.seqNum;
        status = input.status;
        waitingToBeFilled = input.waitingToBeFilled.load();
        numLogsAdded = input.numLogsAdded;
        numLogsFilled = input.numLogsFilled;
        nextOffset = input.nextOffset;
    }

    LogGroupStatus&
    operator=(const LogGroupStatus& input)
    {
        seqNum = input.seqNum;
        status = input.status;
        waitingToBeFilled = input.waitingToBeFilled.load();
        numLogsAdded = input.numLogsAdded;
        numLogsFilled = input.numLogsFilled;
        nextOffset = input.nextOffset;

        return *this;
    }

    void
    Reset(void)
    {
        seqNum = 0;
        status = LogGroupBufferStatus::INIT;
        waitingToBeFilled = false;
        numLogsAdded = 0;
        numLogsFilled = 0;

        nextOffset = 0;
    }
};

class BufferOffsetAllocator : public LogBufferWriteDoneEvent
{
public:
    BufferOffsetAllocator(void);
    virtual ~BufferOffsetAllocator(void);

    void Init(int numLogGroups, uint32_t logGroupSize, LogGroupReleaser* releaser);
    void Reset(void);

    struct OffsetInFile AllocateBuffer(int size);

    virtual void LogFilled(int logGroupId, MapPageList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

    LogGroupBufferStatus GetStatus(int logGroupId);
    uint32_t GetNumLogsAdded(void);

private:
    inline bool _CanAllocate(int logGroupId, int size);

    uint64_t _GetBufferOffsetToWrite(int logGroupId, int size);
    int _GetNewActiveGroup(void);
    uint32_t _GetNextSeqNum(void);
    bool _IsFullyFilled(int logGroupId);
    void _TryToSetFull(int logGroupId);
    void _SetActive(int logGroupId);

    LogGroupReleaser* releaser;

    std::mutex allocateLock;
    std::mutex fullTriggerLock;
    std::vector<LogGroupStatus> statusList;

    std::mutex seqNumberLock;
    uint32_t nextSeqNumber;

    int currentLogGroupId;

    int numLogGroups;
    uint32_t maxOffsetPerGroup;
};
} // namespace ibofos
