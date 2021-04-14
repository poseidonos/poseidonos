
#ifndef AIR_SWITCH_GEAR_COR_HANDLER_H
#define AIR_SWITCH_GEAR_COR_HANDLER_H

#include "src/collection/SwitchGear.h"
#include "src/lib/Design.h"

namespace collection
{
class SwitchGearCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit SwitchGearCoRHandler(SwitchGear* new_switch_gear)
    : switch_gear(new_switch_gear)
    {
    }
    virtual ~SwitchGearCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        switch_gear->Run();
    }

private:
    SwitchGear* switch_gear{nullptr};
};

} // namespace collection

#endif // AIR_SWITCH_GEAR_COR_HANDLER_H
