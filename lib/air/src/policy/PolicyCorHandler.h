
#ifndef AIR_POLICY_COR_HANDLER_H
#define AIR_POLICY_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/policy/PolicyObserver.h"

namespace policy
{
class PolicyCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit PolicyCoRHandler(policy::Observer* new_observer)
    : observer(new_observer)
    {
    }
    virtual ~PolicyCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        observer->Handle();
    }

private:
    policy::Observer* observer{nullptr};
};

} // namespace policy

#endif // AIR_POLICY_COR_HANDLER_H
