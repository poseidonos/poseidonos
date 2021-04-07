
#ifndef AIR_IN_COR_HANDLER_H
#define AIR_IN_COR_HANDLER_H

#include "src/input/In.h"
#include "src/lib/Design.h"

namespace input
{
class InCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit InCoRHandler(InCommand* new_in_command)
    : in_command(new_in_command)
    {
    }
    virtual ~InCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        in_command->HandleKernelMsg();
    }

private:
    InCommand* in_command{nullptr};
};

} // namespace input

#endif // AIR_IN_COR_HANDLER_H
