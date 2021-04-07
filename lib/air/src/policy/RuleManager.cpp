
#include "src/policy/RuleManager.h"

#include "src/config/ConfigInterface.h"

int
policy::Subject::Notify(uint32_t index, uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2, int pid,
    int cmd_type, int cmd_order)
{
    int result = -1;
    if (index < to_dtype(pi::PolicySubject::COUNT))
    {
        arr_observer[index]->Update(type1, type2, value1, value2, pid, cmd_type,
            cmd_order);
        result = 0;
    }

    return result;
}

int
policy::RuleManager::SetNodeMetaConfig(void* node)
{
    int result = 1;
    air::Node* node_meta;
    node_meta = (air::Node*)node;
    uint32_t num_nodes = cfg::GetArrSize(config::ConfigType::NODE);
    for (uint32_t i = 0; i < num_nodes; i++)
    {
        config::String node_name = cfg::GetName(config::ConfigType::NODE, i);
        config::String type =
            cfg::GetStrValue(config::ConfigType::NODE, "Type", node_name);
        bool enable =
            cfg::GetIntValue(config::ConfigType::NODE, "NodeRun", node_name);

        node_meta[i].nid = i;
        if (0 == type.Compare("PERFORMANCE"))
        {
            node_meta[i].processor_type = air::ProcessorType::PERFORMANCE;
        }
        else if (0 == type.Compare("LATENCY"))
        {
            node_meta[i].processor_type = air::ProcessorType::LATENCY;
        }
        else if (0 == type.Compare("QUEUE"))
        {
            node_meta[i].processor_type = air::ProcessorType::QUEUE;
        }
        node_meta[i].enable = enable;

        config::String group_name =
            cfg::GetStrValue(config::ConfigType::NODE, "GroupName", node_name);
        if (group_name != "")
        {
            uint32_t gid = cfg::GetIndex(config::ConfigType::GROUP, group_name);
            node_meta[i].group_id = gid;
        }
    }

    return result;
}

int
policy::RuleManager::SetGlobalConfig(void)
{
    int32_t streaming_interval =
        cfg::GetIntValue(config::ConfigType::DEFAULT, "StreamingInterval");
    ruler->SetStreamingInterval(streaming_interval);
    int32_t aid_size = cfg::GetIntValue(config::ConfigType::DEFAULT, "AidSize");
    ruler->SetAidSize(aid_size);

    return 1;
}

int
policy::RuleManager::UpdateRule(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2, int pid,
    int cmd_type, int cmd_order)
{
    int result = ruler->CheckRule(type1, type2, value1, value2);
    if (0 == result)
    {
        if (ruler->SetRule(type1, type2, value1, value2))
        {
            uint16_t upper_bit = static_cast<uint16_t>(type2 >> 16);

            switch (upper_bit)
            {
                case (to_dtype(pi::Type2_Upper::COLLECTION)):
                {
                    subject->Notify(to_dtype(pi::PolicySubject::TO_COLLECTION),
                        to_dtype(pi::Type1::POLICY_TO_COLLECTION), type2,
                        value1, value2, pid, cmd_type, cmd_order);
                    break;
                }

                case (to_dtype(pi::Type2_Upper::OUTPUT)):
                {
                    subject->Notify(to_dtype(pi::PolicySubject::TO_OUTPUT),
                        to_dtype(pi::Type1::POLICY_TO_OUTPUT), 0, 0, 0, pid,
                        cmd_type, cmd_order);
                    break;
                }
            }

            ruler->SetUpdate(false);
        }
    }
    return result;
}

void
policy::RuleManager::HandleMsg(void)
{
    while (!msg.empty())
    {
        lib::MsgEntry entry = msg.front();
        msg.pop();
        int result =
            UpdateRule(entry.type1, entry.type2, entry.value1, entry.value2,
                entry.pid, entry.cmd_type, entry.cmd_order);
        if (0 > result)
        {
            subject->Notify(to_dtype(pi::PolicySubject::TO_OUTPUT),
                to_dtype(pi::Type1::POLICY_TO_OUTPUT), 0, result * -1, 0,
                entry.pid, entry.cmd_type, entry.cmd_order);
        }
    }
}
