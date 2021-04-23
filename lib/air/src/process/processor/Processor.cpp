
#include "src/process/processor/Processor.h"

#include <string.h>

#include "src/lib/Type.h"
#include "src/lib/json/Json.h"

bool
process::Processor::StreamData(std::string node_name, uint32_t tid,
    const char* tname, node::Thread* thread,
    air::ProcessorType ptype, uint32_t new_time,
    uint32_t max_aid_size)
{
    if (nullptr == thread)
    {
        return false;
    }
    if (false == thread->IsLogging())
    {
        return false;
    }
    time = new_time;

    auto& node = air::json(node_name);

    uint32_t obj_count{0};
    lib::Data* air_data{nullptr};
    lib::AccData* acc_data{nullptr};
    uint64_t aid_value{0};
    for (uint32_t aid_idx = 0; aid_idx < max_aid_size; aid_idx++)
    {
        air_data = thread->GetAirData(aid_idx);
        acc_data = thread->GetAccData(aid_idx);
        if (nullptr != air_data && 0 != air_data->access)
        {
            if (_ProcessData(air_data, acc_data))
            {
                obj_count++;
                aid_value = thread->GetUserAidValue(aid_idx);

                if (air::ProcessorType::PERFORMANCE == ptype)
                {
                    lib::PerformanceData* perf_data =
                        static_cast<lib::PerformanceData*>(air_data);
                    lib::AccPerformanceData* perf_acc =
                        static_cast<lib::AccPerformanceData*>(acc_data);

                    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_" + std::to_string(aid_value));
                    node_obj["target_id"] = {tid};
                    node_obj["target_name"] = {tname};
                    node_obj["app_id"] = {aid_value};
                    node_obj["iops_read"] = {perf_data->iops_read};
                    node_obj["iops_write"] = {perf_data->iops_write};
                    node_obj["iops_total"] = {perf_data->iops_total};
                    node_obj["bw_read"] = {perf_data->bandwidth_read};
                    node_obj["bw_write"] = {perf_data->bandwidth_write};
                    node_obj["bw_total"] = {perf_data->bandwidth_total};
                    node_obj["iops_read_avg"] = {perf_acc->iops_read_avg};
                    node_obj["iops_write_avg"] = {perf_acc->iops_write_avg};
                    node_obj["bw_read_avg"] = {perf_acc->bandwidth_read_avg};
                    node_obj["bw_write_avg"] = {perf_acc->bandwidth_write_avg};
                    uint32_t cnt = 1;
                    for (const auto& pair : perf_data->packet_cnt)
                    {
                        node_obj["cnt_" + std::to_string(cnt)] = {
                            std::to_string(pair.first) + "(sz)-" +
                            std::to_string(pair.second) + "(cnt)"};
                        cnt++;
                    }

                    node["objs"] += {node_obj};
                }
                else if (air::ProcessorType::QUEUE == ptype)
                {
                    lib::QueueData* q_data = static_cast<lib::QueueData*>(air_data);
                    lib::AccQueueData* q_acc = static_cast<lib::AccQueueData*>(acc_data);

                    auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_" + std::to_string(aid_value));
                    node_obj["app_id"] = {aid_value};
                    node_obj["target_id"] = {tid};
                    node_obj["target_name"] = {tname};
                    node_obj["size"] = {q_data->q_size};
                    node_obj["num_req"] = {q_data->num_req};
                    node_obj["depth_period_avg"] = {q_data->depth_period_avg};
                    node_obj["depth_period_max"] = {q_data->depth_period_max};
                    node_obj["depth_total_avg"] = {q_acc->depth_total_avg};
                    node_obj["depth_total_max"] = {q_acc->depth_total_max};

                    node["objs"] += {node_obj};
                }
                else if (air::ProcessorType::UTILIZATION == ptype)
                {
                    lib::UtilizationData* util_data = static_cast<lib::UtilizationData*>(air_data);
                    lib::AccUtilizationData* util_acc = static_cast<lib::AccUtilizationData*>(acc_data);

                    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
                    {
                        auto& node_obj = air::json(node_name + "_" + "util" + std::to_string(tid) + "_" + std::to_string(aid_value) + "_" + std::to_string(idx));
                        node_obj["target_id"] = {tid};
                        node_obj["target_name"] = {tname};
                        node_obj["app_id"] = {aid_value};
                        node_obj["enum_id"] = {idx};
                        node_obj["enum_name"] = {"TBD"};
                        node_obj["util"] = {util_data->percent[idx]};
                        node_obj["total_util"] = {util_acc->total_percent[idx]};

                        node["objs"] += {node_obj};
                    }
                }
                else if (air::ProcessorType::COUNT == ptype)
                {
                    lib::CountData* count_data = static_cast<lib::CountData*>(air_data);
                    lib::AccCountData* count_acc = static_cast<lib::AccCountData*>(acc_data);

                    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
                    {
                        auto& node_obj = air::json(node_name + "_" + "count" + std::to_string(tid) + "_" + std::to_string(aid_value) + "_" + std::to_string(idx));
                        node_obj["enum_id"] = {idx};
                        node_obj["enum_name"] = {"TBD"};
                        node_obj["target_id"] = {tid};
                        node_obj["target_name"] = {tname};
                        node_obj["app_id"] = {aid_value};
                        node_obj["count"] = {count_data->count[idx]};
                        node_obj["num_req"] = {count_data->num_req[idx]};
                        node_obj["total_count"] = {count_acc->total_count[idx]};
                        node_obj["total_num_req"] = {count_acc->total_num_req[idx]};
                        node_obj["total_count_avg"] = {count_acc->total_count_avg[idx]};

                        node["objs"] += {node_obj};
                    }
                }

                _InitData(air_data, acc_data);
            }
        }
        thread->SwapBuffer(aid_idx);
    }

    return true;
}

bool
process::Processor::StreamData(std::string node_name,
    lib::AccLatencyData* data, uint32_t aid)
{
    if (nullptr == data)
    {
        return false;
    }

    bool result{false};
    result = _ProcessData(nullptr, data);

    if (result)
    {
        auto& node = air::json(node_name);
        for (uint32_t sid = 0; sid < lib::SID_SIZE; sid++)
        {
            lib::AccLatencySeqData* lat_data = &(data->seq_data[sid]);
            if (0 != lat_data->total_sample_count)
            {
                auto& node_obj = air::json(node_name + "_" + std::to_string(aid) + "_" + std::to_string(sid));
                node_obj["target_id"] = {nullptr};
                node_obj["target_name"] = {std::to_string(sid) + "-" + std::to_string(sid + 1)};
                node_obj["app_id"] = {aid};
                node_obj["mean"] = {lat_data->mean};
                node_obj["min"] = {lat_data->min};
                node_obj["max"] = {lat_data->max};
                node_obj["median"] = {lat_data->median};
                node_obj["low_qt"] = {lat_data->lower_quartile};
                node_obj["up_qt"] = {lat_data->upper_quartile};
                node_obj["sample_cnt"] = {lat_data->sample_count};
                node_obj["bucket_cnt"] = {lat_data->bucket_count};
                node_obj["total_mean"] = {lat_data->total_mean};
                node_obj["total_min"] = {lat_data->total_min};
                node_obj["total_max"] = {lat_data->total_max};
                node_obj["total_median"] = {lat_data->total_median};
                node_obj["total_low_qt"] = {lat_data->total_lower_quartile};
                node_obj["total_up_qt"] = {lat_data->total_upper_quartile};
                node_obj["total_sample_cnt"] = {lat_data->total_sample_count};

                node["objs"] += {node_obj};
            }
        }
    }

    _InitData(nullptr, data);

    return result;
}
