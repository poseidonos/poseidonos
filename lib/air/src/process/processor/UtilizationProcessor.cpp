
#include "src/process/processor/UtilizationProcessor.h"

#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/json/Json.h"

void
process::UtilizationProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::UtilizationData* air_util_data = static_cast<lib::UtilizationData*>(air_data);
    lib::AccUtilizationData* acc_util_data = static_cast<lib::AccUtilizationData*>(acc_data);

    acc_util_data->total_usage += air_util_data->usage;
}

void
process::UtilizationProcessor::_JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
    air::string_view& node_name_view, uint32_t tid, const char* tname,
    uint64_t hash_value, uint32_t filter_index)
{
    lib::UtilizationData* air_util_data = static_cast<lib::UtilizationData*>(air_data);
    std::string node_name;
    node_name.assign(node_name_view.data(), node_name_view.size());
    auto& node = air::json(node_name);
    lib::AccUtilizationData* acc_util_data = static_cast<lib::AccUtilizationData*>(acc_data);

    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_util_" + std::to_string(hash_value) + "_" + std::to_string(filter_index));

    std::string filter_item = cfg::GetItemStrWithNodeName(node_name_view, filter_index);

    node_obj["usage"] = {air_util_data->usage};
    node_obj["total_usage"] = {acc_util_data->total_usage};

    node_obj["target_id"] = {tid};
    node_obj["target_name"] = {tname};
    node_obj["index"] = {hash_value};
    node_obj["filter"] = {filter_item};

    node["objs"] += {node_obj};
}

void
process::UtilizationProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccUtilizationData* acc_util_data = static_cast<lib::AccUtilizationData*>(acc_data);
    lib::UtilizationData* air_util_data = static_cast<lib::UtilizationData*>(air_data);

    air_util_data->access = 0;
    air_util_data->usage = 0;

    if (0 != acc_util_data->need_erase)
    {
        acc_util_data->total_usage = 0;
        acc_util_data->need_erase = 0;
    }
}
