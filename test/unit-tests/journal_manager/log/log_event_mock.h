#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/log_event.h"

namespace pos
{
class MockLog : public Log
{
public:
    using Log::Log;
};

class MockBlockWriteDoneLog : public BlockWriteDoneLog
{
public:
    using BlockWriteDoneLog::BlockWriteDoneLog;
};

class MockStripeMapUpdatedLog : public StripeMapUpdatedLog
{
public:
    using StripeMapUpdatedLog::StripeMapUpdatedLog;
};

class MockGcBlockWriteDoneLog : public GcBlockWriteDoneLog
{
public:
    using GcBlockWriteDoneLog::GcBlockWriteDoneLog;
};

class MockGcStripeFlushedLog : public GcStripeFlushedLog
{
public:
    using GcStripeFlushedLog::GcStripeFlushedLog;
};

class MockVolumeDeletedLog : public VolumeDeletedLog
{
public:
    using VolumeDeletedLog::VolumeDeletedLog;
};

class MockSegmentFreeddLog : public SegmentFreedLog
{
public:
    using SegmentFreedLog::SegmentFreedLog;
};

} // namespace pos
