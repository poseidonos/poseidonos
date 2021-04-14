
#ifndef AIR_PROCESS_COR_HANDLER_H
#define AIR_PROCESS_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/process/ProcessManager.h"

namespace process
{
class ProcessCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit ProcessCoRHandler(ProcessManager* new_process_manager)
    : process_manager(new_process_manager)
    {
    }
    virtual ~ProcessCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        process_manager->StreamData();
        process_manager->SetTimeSlot();
    }

private:
    ProcessManager* process_manager{nullptr};
};

} // namespace process

#endif // AIR_PROCESS_COR_HANDLER_H
