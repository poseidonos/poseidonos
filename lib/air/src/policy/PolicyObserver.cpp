
#include "src/policy/PolicyObserver.h"

void
policy::Observer::Update(uint32_t type1, uint32_t type2, uint32_t value1,
    uint32_t value2, int pid, int cmd_type,
    int cmd_order)
{
    rule_manager->EnqueueMsg(type1, type2, value1, value2, pid, cmd_type,
        cmd_order);
}

void
policy::Observer::Handle(void)
{
    rule_manager->HandleMsg();
}
