
#ifndef AIR_POLICY_OBSERVER_H
#define AIR_POLICY_OBSERVER_H

#include "src/lib/Design.h"
#include "src/lib/Protocol.h"
#include "src/policy/RuleManager.h"

namespace policy
{
class Observer : public lib_design::Observer
{
public:
    explicit Observer(RuleManager* new_rule_manager)
    : rule_manager(new_rule_manager)
    {
    }
    virtual ~Observer(void)
    {
    }
    virtual void Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order);
    virtual void Handle(void);

private:
    RuleManager* rule_manager{nullptr};
};

} // namespace policy

#endif // AIR_POLICY_OBSERVER_H
