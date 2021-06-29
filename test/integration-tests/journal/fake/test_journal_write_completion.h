#pragma once

#include "src/event_scheduler/event.h"

namespace pos
{
class WrittenLogs;

class TestJournalWriteCompletion : public Event
{
public:
    explicit TestJournalWriteCompletion(WrittenLogs* logs);
    bool Execute(void) override;

private:
    WrittenLogs* logs;
};
} // namespace pos
