
#include "src/process/processor/CountProcessor.h"

#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/StringView.h"
#include "src/lib/json/Json.h"

void
process::CountProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::CountData* air_count_data = static_cast<lib::CountData*>(air_data);
    lib::AccCountData* acc_count_data = static_cast<lib::AccCountData*>(acc_data);

    if (air_count_data->count_positive > air_count_data->count_negative)
    {
        air_count_data->count = air_count_data->count_positive - air_count_data->count_negative;
        air_count_data->negative = 0;
    }
    else
    {
        air_count_data->count = air_count_data->count_negative - air_count_data->count_positive;
        air_count_data->negative = 1;
    }

    acc_count_data->total_num_req_positive += air_count_data->num_req_positive;
    acc_count_data->total_num_req_negative += air_count_data->num_req_negative;

    if (air_count_data->negative == acc_count_data->negative)
    {
        acc_count_data->total_count += air_count_data->count;
    }
    else
    {
        if (air_count_data->count > acc_count_data->total_count)
        {
            acc_count_data->negative = air_count_data->negative;
            acc_count_data->total_count = air_count_data->count - acc_count_data->total_count;
        }
        else
        {
            acc_count_data->total_count -= air_count_data->count;
        }
    }
}

void
process::CountProcessor::_JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
    air::string_view& node_name_view, uint32_t tid, const char* tname,
    uint64_t hash_value, uint32_t filter_index)
{
    lib::CountData* air_count_data = static_cast<lib::CountData*>(air_data);
    std::string node_name;
    node_name.assign(node_name_view.data(), node_name_view.size());
    auto& node = air::json(node_name);
    lib::AccCountData* acc_count_data = static_cast<lib::AccCountData*>(acc_data);

    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_count_" + std::to_string(hash_value) + "_" + std::to_string(filter_index));

    std::string filter_item = cfg::GetItemStrWithNodeName(node_name_view, filter_index);

    node_obj["target_id"] = {tid};
    node_obj["target_name"] = {tname};
    node_obj["index"] = {hash_value};
    node_obj["filter"] = {filter_item};

    if (0 == air_count_data->negative)
    {
        node_obj["count"] = {air_count_data->count};
    }
    else
    {
        node_obj["count"] = {((int64_t)(air_count_data->count)) * -1};
    }
    node_obj["count_plus"] = {air_count_data->count_positive};
    node_obj["count_minus"] = {air_count_data->count_negative};
    node_obj["num_req_plus"] = {air_count_data->num_req_positive};
    node_obj["num_req_minus"] = {air_count_data->num_req_negative};
    node_obj["negative"] = {air_count_data->negative};

    if (0 == acc_count_data->negative)
    {
        node_obj["total_count"] = {acc_count_data->total_count};
    }
    else
    {
        node_obj["total_count"] = {((int64_t)(acc_count_data->total_count)) * -1};
    }
    node_obj["total_num_req_plus"] = {acc_count_data->total_num_req_positive};
    node_obj["total_num_req_minus"] = {acc_count_data->total_num_req_negative};
    node_obj["total_negative"] = {acc_count_data->negative};

    node["objs"] += {node_obj};
}

void
process::CountProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccCountData* acc_count_data = static_cast<lib::AccCountData*>(acc_data);
    lib::CountData* air_count_data = static_cast<lib::CountData*>(air_data);

    air_count_data->access = 0;
    air_count_data->count_positive = 0;
    air_count_data->count_negative = 0;
    air_count_data->num_req_positive = 0;
    air_count_data->num_req_negative = 0;
    air_count_data->count = 0;
    air_count_data->negative = 0;

    if (0 != acc_count_data->need_erase)
    {
        acc_count_data->total_count = 0;
        acc_count_data->total_num_req_positive = 0;
        acc_count_data->total_num_req_negative = 0;
        acc_count_data->negative = 0;
        acc_count_data->need_erase = 0;
    }
}
