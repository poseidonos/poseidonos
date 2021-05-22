
#include "src/process/processor/LatencyProcessor.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/json/Json.h"

void
process::LatencyProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccLatencyData* acc_lat_data = static_cast<lib::AccLatencyData*>(acc_data);
    _Calculate(acc_lat_data);
}

void
process::LatencyProcessor::_Calculate(lib::AccLatencyData* lat_data)
{
    if (1 > lat_data->sample_count)
    {
        return;
    }
    uint32_t sample_count = lat_data->sample_count;

    // calculate period statistics
    std::sort(lat_data->timelag, lat_data->timelag + sample_count);
    uint64_t sum_value = 0;
    for (uint32_t sample_index = 0; sample_index < sample_count; sample_index++)
    {
        sum_value += lat_data->timelag[sample_index];
    }
    lat_data->mean = (uint32_t)(sum_value / sample_count);
    lat_data->median = lat_data->timelag[sample_count / 2];
    lat_data->lower_quartile = lat_data->timelag[sample_count / 4];
    lat_data->upper_quartile = lat_data->timelag[sample_count - (sample_count / 4)];

    // calculate total statistics
    uint64_t total_count =
        lat_data->total_sample_count + (uint64_t)lat_data->sample_count;
    if (total_count >= OVERFLOW_THRESHOLD)
    {
        lat_data->overflow_warning = true;
        return;
    }
    if (0 < lat_data->total_sample_count)
    {
        double divisor1 =
            (double)(lat_data->total_sample_count) /
            (double)(lat_data->total_sample_count + lat_data->sample_count);
        double divisor2 =
            (double)(lat_data->sample_count) /
            (double)(lat_data->total_sample_count + lat_data->sample_count);
        lat_data->total_mean =
            (double)(lat_data->total_mean * divisor1) + (double)(lat_data->mean * divisor2);
        lat_data->total_median =
            (double)(lat_data->total_median * divisor1) + (double)(lat_data->median * divisor2);
        lat_data->total_lower_quartile =
            (double)(lat_data->total_lower_quartile * divisor1) +
            (double)(lat_data->lower_quartile * divisor2);
        lat_data->total_upper_quartile =
            (double)(lat_data->total_upper_quartile * divisor1) +
            (double)(lat_data->upper_quartile * divisor2);
    }
    else
    {
        lat_data->total_mean = lat_data->mean;
        lat_data->total_median = lat_data->median;
        lat_data->total_lower_quartile = lat_data->lower_quartile;
        lat_data->total_upper_quartile = lat_data->upper_quartile;
    }

    lat_data->total_sample_count = total_count;
    if (lat_data->max > lat_data->total_max)
    {
        lat_data->total_max = lat_data->max;
    }
    if ((0 != lat_data->min) &&
        ((0 == lat_data->total_min) || (lat_data->min < lat_data->total_min)))
    {
        lat_data->total_min = lat_data->min;
    }
}

void
process::LatencyProcessor::_JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
    air::string_view& node_name_view, uint32_t tid, const char* tname,
    uint64_t hash_value, uint32_t filter_index)
{
    lib::AccLatencyData* acc_lat_data = static_cast<lib::AccLatencyData*>(acc_data);
    std::string node_name;
    node_name.assign(node_name_view.data(), node_name_view.size());
    auto& node = air::json(node_name);

    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_lat_" + std::to_string(hash_value) + "_" + std::to_string(filter_index));

    std::string filter_range = cfg::GetItemStrWithNodeName(node_name_view, filter_index);
    filter_range += "~";
    filter_range += cfg::GetItemStrWithNodeName(node_name_view, filter_index + 1);

    node_obj["index"] = {hash_value};
    node_obj["filter"] = {filter_range};
    node_obj["target_id"] = {tid};
    node_obj["target_name"] = {tname};

    node_obj["mean"] = {acc_lat_data->mean};
    node_obj["min"] = {acc_lat_data->min};
    node_obj["max"] = {acc_lat_data->max};
    node_obj["median"] = {acc_lat_data->median};
    node_obj["low_qt"] = {acc_lat_data->lower_quartile};
    node_obj["up_qt"] = {acc_lat_data->upper_quartile};
    node_obj["sample_cnt"] = {acc_lat_data->sample_count};
    node_obj["total_mean"] = {acc_lat_data->total_mean};
    node_obj["total_min"] = {acc_lat_data->total_min};
    node_obj["total_max"] = {acc_lat_data->total_max};
    node_obj["total_median"] = {acc_lat_data->total_median};
    node_obj["total_low_qt"] = {acc_lat_data->total_lower_quartile};
    node_obj["total_up_qt"] = {acc_lat_data->total_upper_quartile};
    node_obj["total_sample_cnt"] = {acc_lat_data->total_sample_count};

    node["objs"] += {node_obj};
}

void
process::LatencyProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccLatencyData* acc_lat_data = static_cast<lib::AccLatencyData*>(acc_data);

    acc_lat_data->mean = 0;
    acc_lat_data->min = 0;
    acc_lat_data->max = 0;
    acc_lat_data->median = 0;
    acc_lat_data->lower_quartile = 0;
    acc_lat_data->upper_quartile = 0;
    acc_lat_data->sample_count = 0;
    for (uint32_t i = 0; i < lib::TIMELAG_SIZE; i++)
    {
        acc_lat_data->timelag[i] = 0;
    }

    if ((true == acc_lat_data->overflow_warning) ||
        (0 != acc_lat_data->need_erase))
    {
        acc_lat_data->total_mean = 0;
        acc_lat_data->total_min = 0;
        acc_lat_data->total_max = 0;
        acc_lat_data->total_median = 0;
        acc_lat_data->total_lower_quartile = 0;
        acc_lat_data->total_upper_quartile = 0;
        acc_lat_data->total_sample_count = 0;
        acc_lat_data->overflow_warning = 0;
        acc_lat_data->need_erase = 0;
    }
}
