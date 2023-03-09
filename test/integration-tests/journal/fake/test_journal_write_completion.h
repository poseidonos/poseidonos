#pragma once

#include "src/event_scheduler/meta_update_call_back.h"
#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/log/log_event.h"

namespace pos
{
class SegmentContextUpdater;
class VSAMapFake;
class WrittenLogs;

class TestJournalWriteCompletion : public MetaUpdateCallback
{
public:
    TestJournalWriteCompletion(VolumeIoSmartPtr volumeIo, SegmentContextUpdater* segmentContextUpdater, VSAMapFake* vsaMap, WrittenLogs* logs, LogType eventType);

private:
    bool _DoSpecificJob(void) override;
    void _InvalidateOldBlock(void);
    void _UpdateBlockMap(void);

    VolumeIoSmartPtr volumeIo;
    SegmentContextUpdater* segmentContextUpdater;
    VSAMapFake* vsaMap;
    WrittenLogs* logs;
    LogType eventType;
};
} // namespace pos
