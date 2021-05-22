
#include "src/process/processor/QueueProcessor.h"

#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/json/Json.h"

void
process::QueueProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);
    lib::AccQueueData* acc_queue_data = static_cast<lib::AccQueueData*>(acc_data);

    if (air_queue_data->num_req > 0)
    {
        air_queue_data->depth_period_avg =
            (float)air_queue_data->sum_depth / air_queue_data->num_req;
    }
    if (acc_queue_data->depth_total_max < air_queue_data->depth_period_max)
    {
        acc_queue_data->depth_total_max = air_queue_data->depth_period_max;
    }

    uint32_t prev_total_num_req = acc_queue_data->total_num_req;
    acc_queue_data->total_num_req += air_queue_data->num_req;

    if (0 != acc_queue_data->total_num_req)
    {
        acc_queue_data->depth_total_avg =
            (acc_queue_data->depth_total_avg / acc_queue_data->total_num_req) * prev_total_num_req +
            (air_queue_data->depth_period_avg / acc_queue_data->total_num_req) * air_queue_data->num_req;
    }
}

void
process::QueueProcessor::_JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
    air::string_view& node_name_view, uint32_t tid, const char* tname,
    uint64_t hash_value, uint32_t filter_index)
{
    lib::AccQueueData* acc_queue_data = static_cast<lib::AccQueueData*>(acc_data);
    std::string node_name;
    node_name.assign(node_name_view.data(), node_name_view.size());
    auto& node = air::json(node_name);
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);

    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_queue_" + std::to_string(hash_value) + "_" + std::to_string(filter_index));

    std::string filter_item = cfg::GetItemStrWithNodeName(node_name_view, filter_index);

    node_obj["filter"] = {filter_item};
    node_obj["target_id"] = {tid};
    node_obj["index"] = {hash_value};
    node_obj["target_name"] = {tname};

    node_obj["num_req"] = {air_queue_data->num_req};
    node_obj["depth_period_avg"] = {air_queue_data->depth_period_avg};
    node_obj["depth_period_max"] = {air_queue_data->depth_period_max};

    node_obj["depth_total_avg"] = {acc_queue_data->depth_total_avg};
    node_obj["depth_total_max"] = {acc_queue_data->depth_total_max};

    node["objs"] += {node_obj};
}

void
process::QueueProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccQueueData* acc_queue_data = static_cast<lib::AccQueueData*>(acc_data);
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);

    air_queue_data->access = 0;
    air_queue_data->num_req = 0;
    air_queue_data->sum_depth = 0;
    air_queue_data->depth_period_avg = 0.0;
    air_queue_data->depth_period_max = 0;

    if (0 != acc_queue_data->need_erase)
    {
        acc_queue_data->depth_total_max = 0;
        acc_queue_data->total_num_req = 0;
        acc_queue_data->depth_total_avg = 0.0;
        acc_queue_data->need_erase = 0;
    }
}
