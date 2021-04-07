#include "src/process/ProcessManager.h"

#include <time.h>

#include "src/config/ConfigInterface.h"

process::ProcessManager::~ProcessManager(void)
{
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        if (nullptr != processor[i])
        {
            delete processor[i];
        }
    }
}

int
process::ProcessManager::Init(void)
{
    int num_nodes = 0;
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        switch (node_meta_getter->NodeProcessorType(i))
        {
            case (air::ProcessorType::PERFORMANCE):
                processor[i] = new PerformanceProcessor;
                break;

            case (air::ProcessorType::LATENCY):
                processor[i] = new LatencyProcessor;
                break;

            case (air::ProcessorType::QUEUE):
                processor[i] = new QueueProcessor;
                break;

            default:
                break;
        }
        num_nodes++;
    }
    return num_nodes;
}

void
process::ProcessManager::SetTimeSlot(void)
{
    timing_distributor.SetTiming(node_meta_getter, global_meta_getter,
        node_manager);
}

void
process::ProcessManager::StreamData(void)
{
    auto& air_json = air::json("air");
    time_t curr_time = time(NULL);

    air_json["timestamp"] = {(int64_t)curr_time};
    air_json["interval"] = {global_meta_getter->StreamingInterval()};
    air_json["play"] = {global_meta_getter->Enable()};
    air_json["group"];

    _AddGroupInfo(air_json);

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        air::ProcessorType type = node_meta_getter->NodeProcessorType(i);
        if (air::ProcessorType::PROCESSORTYPE_NULL != type)
        {
            int32_t group_name_leng =
                cfg::GetStrValue(config::ConfigType::NODE, "GroupName",
                    cfg::GetName(config::ConfigType::NODE, i))
                    .Size();
            config::String config_group_name(
                cfg::GetStrValue(config::ConfigType::NODE, "GroupName",
                    cfg::GetName(config::ConfigType::NODE, i))
                    .Data(),
                group_name_leng + 1);
            std::string group_name(config_group_name.Data(), group_name_leng);

            int32_t node_name_leng = cfg::GetName(config::ConfigType::NODE, i).Size();
            std::string node_name(cfg::GetName(config::ConfigType::NODE, i).Data(),
                node_name_leng);

            _AddNodeInfo(group_name, node_name, i, type);
        }
    }
}

void
process::ProcessManager::_AddGroupInfo(air::JSONdoc& doc)
{
    uint32_t group_size = cfg::GetArrSize(config::ConfigType::GROUP);
    for (uint32_t i = 0; i < group_size; i++)
    {
        int32_t group_name_leng = cfg::GetName(config::ConfigType::GROUP, i).Size();
        std::string group_name(cfg::GetName(config::ConfigType::GROUP, i).Data(),
            group_name_leng);

        auto& group_obj = air::json(group_name);
        int32_t group_id = cfg::GetIndex(
            config::ConfigType::GROUP, cfg::GetName(config::ConfigType::GROUP, i));
        group_obj["group_id"] = {group_id};
        group_obj["node"];
        doc["group"][group_name] = {group_obj};
    }
}

void
process::ProcessManager::_AddNodeInfo(std::string& group_name,
    std::string& node_name, uint32_t nid,
    air::ProcessorType type)
{
    uint32_t time = global_meta_getter->StreamingInterval();
    uint32_t max_aid_size = global_meta_getter->AidSize();

    auto& node_obj = air::json(node_name);
    node_obj["node_id"] = {nid};
    bool node_build =
        (bool)cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild",
            cfg::GetName(config::ConfigType::NODE, nid));
    node_obj["build"] = {node_build};
    node_obj["run"] = {node_meta_getter->NodeEnable(nid)};
    if (air::ProcessorType::PERFORMANCE == type)
    {
        node_obj["type"] = {"performance"};
    }
    else if (air::ProcessorType::LATENCY == type)
    {
        node_obj["type"] = {"latency"};
    }
    else if (air::ProcessorType::QUEUE == type)
    {
        node_obj["type"] = {"queue"};
    }
    node_obj["objs"] += {nullptr};
    node_obj["objs"] = {};

    // processing here !!!!
    if (global_meta_getter->Enable() && node_build && node_meta_getter->NodeEnable(nid))
    {
        if (air::ProcessorType::LATENCY == type)
        {
            for (uint32_t aid = 0; aid < max_aid_size; aid++)
            {
                processor[nid]->StreamData(node_name,
                    node_manager->GetAccLatData(nid, aid), aid);
            }
        }
        else if (air::ProcessorType::PERFORMANCE == type || air::ProcessorType::QUEUE == type)
        {
            for (auto it = node_manager->thread_map.begin(); it != node_manager->thread_map.end(); it++)
            {
                node::ThreadArray* arr = &(it->second);
                node::Thread* thread = arr->node[nid];
                processor[nid]->StreamData(node_name, it->first, arr->tname.c_str(),
                    thread, type, time, max_aid_size);
            }
        }
    }

    auto& group_obj = air::json(group_name);
    group_obj["node"][node_name] = {air::json(node_name)};
}
