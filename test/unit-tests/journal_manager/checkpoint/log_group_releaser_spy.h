#pragma once

#include "src/journal_manager/checkpoint/log_group_releaser.h"

namespace pos
{
class LogGroupReleaserSpy : public LogGroupReleaser
{
public:
    LogGroupReleaserSpy(void) = default;
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
