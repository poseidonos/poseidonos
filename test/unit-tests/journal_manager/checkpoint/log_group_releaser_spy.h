#pragma once

#include "src/journal_manager/checkpoint/log_group_releaser.h"

#include <list>

namespace pos
{
class LogGroupReleaserSpy : public LogGroupReleaser
{
public:
    using LogGroupReleaser::LogGroupReleaser;
    virtual ~LogGroupReleaserSpy(void) = default;

    void
    UpdateFlushingLogGroup(void)
    {
        LogGroupReleaser::_UpdateFlushingLogGroup();
    }

    void
    SetFlushingLogGroupId(int id)
    {
        flushingLogGroupId = id;
    }

    bool triggerCheckpoint = true;

    // Metohds to inject protected member values for unit testing
    void
    SetFullLogGroups(std::list<int> logGroups)
    {
        fullLogGroup = logGroups;
    }

    void
    SetCheckpointTriggerInProgress(bool value)
    {
        checkpointTriggerInProgress = value;
    }

    // Method to access protected method of LogGroupReleaser for unit testing
    void
    FlushNextLogGroup(void)
    {
        LogGroupReleaser::_FlushNextLogGroup();
    }

    void
    LogGroupResetCompleted(int logGroupId)
    {
        LogGroupReleaser::_LogGroupResetCompleted(logGroupId);
    }

protected:
    virtual void
    _FlushNextLogGroup(void) override
    {
        if (triggerCheckpoint == true)
        {
            LogGroupReleaser::_FlushNextLogGroup();
        }
    }
};
} // namespace pos
