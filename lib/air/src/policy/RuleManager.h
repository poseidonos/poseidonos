
#ifndef AIR_RULE_MANAGER_H
#define AIR_RULE_MANAGER_H

#include <queue>
#include <string>

#include "src/lib/Design.h"
#include "src/lib/Msg.h"
#include "src/lib/Protocol.h"
#include "src/lib/Type.h"
#include "src/policy/Ruler.h"

namespace policy
{
class Subject : public lib_design::Subject
{
public:
    virtual ~Subject(void)
    {
    }
    virtual int Notify(uint32_t index, uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2, int pid, int cmd_type,
        int cmd_order);
};

class RuleManager
{
public:
    RuleManager(Ruler* new_ruler, policy::Subject* new_subject)
    : ruler(new_ruler),
      subject(new_subject)
    {
    }
    void SetNodeMetaConfig(void* node);
    void SetGlobalConfig(void);
    int UpdateRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order);
    void HandleMsg(void);
    void
    EnqueueMsg(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order)
    {
        msg.push({type1, type2, value1, value2, pid, cmd_type, cmd_order});
    }

private:
    Ruler* ruler{nullptr};
    Subject* subject{nullptr};
    std::queue<lib::MsgEntry> msg;
    //    bool node_link {false};
    //    bool event_link {false};

    //    int pid_        {-1};
    //    int cmd_type_   {-1};
};

} // namespace policy

#endif // AIR_RULE_MANAGER_H
