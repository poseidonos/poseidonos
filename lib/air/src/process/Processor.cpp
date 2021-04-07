
#include "src/process/Processor.h"

#include <string.h>

#include <algorithm>
#include <cmath>

#include "src/lib/Type.h"
#include "src/lib/json/Json.h"

bool
process::Processor::IsIdle(lib::Data* data)
{
    data->idle_count++;

    if (lib::IDLE_THRESHOLD <= data->idle_count)
    {
        return true;
    }
    else
    {
        return false;
    }
}

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
    uint32_t aid_value{0};
    for (uint32_t aid_idx = 0; aid_idx < max_aid_size; aid_idx++)
    {
        air_data = thread->GetAirData(aid_idx);
        acc_data = thread->GetAccData(aid_idx);
        if (nullptr != air_data && air_data->access == 1)
        {
            if (_ProcessData(air_data, acc_data))
            {
                obj_count++;
                aid_value = thread->GetUserAidValue(aid_idx);

                auto& node_obj = air::json(node_name + "_" + std::to_string(tid) + "_" + std::to_string(aid_value));
                node_obj["target_id"] = {tid};
                node_obj["target_name"] = {tname};
                node_obj["app_id"] = {aid_value};
                if (air::ProcessorType::PERFORMANCE == ptype)
                {
                    lib::PerformanceData* perf_data =
                        static_cast<lib::PerformanceData*>(air_data);
                    lib::AccPerformanceData* perf_acc =
                        static_cast<lib::AccPerformanceData*>(acc_data);
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
                }
                else if (air::ProcessorType::QUEUE == ptype)
                {
                    lib::QueueData* q_data = static_cast<lib::QueueData*>(air_data);
                    lib::AccQueueData* q_acc = static_cast<lib::AccQueueData*>(acc_data);
                    node_obj["size"] = {q_data->q_size};
                    node_obj["num_req"] = {q_data->num_req};
                    node_obj["depth_period_avg"] = {q_data->depth_period_avg};
                    node_obj["depth_period_max"] = {q_data->depth_period_max};
                    node_obj["depth_total_avg"] = {q_acc->depth_total_avg};
                    node_obj["depth_total_max"] = {q_acc->depth_total_max};
                }

                node["objs"] += {node_obj};

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

bool
process::PerformanceProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(air_data);
    lib::AccPerformanceData* Acc_perf_data =
        static_cast<lib::AccPerformanceData*>(acc_data);

    perf_data->iops_total = perf_data->iops_read + perf_data->iops_write;
    perf_data->bandwidth_total =
        perf_data->bandwidth_read + perf_data->bandwidth_write;
    if (time > 1)
    {
        perf_data->iops_read = perf_data->iops_read / time;
        perf_data->iops_write = perf_data->iops_write / time;
        perf_data->iops_total = perf_data->iops_total / time;
        perf_data->bandwidth_read = perf_data->bandwidth_read / time;
        perf_data->bandwidth_write = perf_data->bandwidth_write / time;
        perf_data->bandwidth_total = perf_data->bandwidth_total / time;
    }
    if (time > 0)
    {
        double common_divisor = (double)(Acc_perf_data->time_spent + time);

        Acc_perf_data->iops_read_avg =
            ((double)Acc_perf_data->iops_read_avg / common_divisor *
                Acc_perf_data->time_spent) +
            ((double)perf_data->iops_read / common_divisor * time);

        Acc_perf_data->iops_write_avg =
            ((double)Acc_perf_data->iops_write_avg / common_divisor *
                Acc_perf_data->time_spent) +
            ((double)perf_data->iops_write / common_divisor * time);

        Acc_perf_data->bandwidth_read_avg =
            ((double)Acc_perf_data->bandwidth_read_avg / common_divisor *
                Acc_perf_data->time_spent) +
            ((double)perf_data->bandwidth_read / common_divisor * time);

        Acc_perf_data->bandwidth_write_avg =
            ((double)Acc_perf_data->bandwidth_write_avg / common_divisor *
                Acc_perf_data->time_spent) +
            ((double)perf_data->bandwidth_write / common_divisor * time);
    }
    Acc_perf_data->time_spent += time;

    return true;
}

void
process::PerformanceProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(air_data);
    lib::AccPerformanceData* acc_perf_data =
        static_cast<lib::AccPerformanceData*>(acc_data);

    perf_data->bandwidth_read = 0;
    perf_data->bandwidth_write = 0;
    perf_data->bandwidth_total = 0;
    perf_data->iops_read = 0;
    perf_data->iops_write = 0;
    perf_data->iops_total = 0;
    perf_data->packet_cnt.clear();
    perf_data->access = 0;

    if (1 == acc_perf_data->need_erase)
    {
        acc_perf_data->bandwidth_read_avg = 0;
        acc_perf_data->bandwidth_write_avg = 0;
        acc_perf_data->iops_read_avg = 0;
        acc_perf_data->iops_write_avg = 0;
        acc_perf_data->time_spent = 0;
        acc_perf_data->need_erase = 0;
    }
}

bool
process::LatencyProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccLatencyData* acc_lat_data =
        static_cast<lib::AccLatencyData*>(acc_data);
    uint32_t max_sid_size = lib::SID_SIZE;
    uint32_t total_sample_count = 0;

    for (uint32_t sid = 0; sid < max_sid_size - 1; sid++)
    {
        lib::AccLatencySeqData* lat_data = &(acc_lat_data->seq_data[sid]);
        _Calculate(lat_data);
        total_sample_count += lat_data->total_sample_count;
    }

    if (0 != total_sample_count)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void
process::LatencyProcessor::_Calculate(lib::AccLatencySeqData* lat_data)
{
    if (lat_data->sample_count < 1)
    {
        return;
    }
    // calculate each buckets
    for (auto bucket : lat_data->bucket)
    {
        uint32_t time_lag_size = bucket->time_lag_size;
        uint64_t sum_value = 0;

        std::sort(bucket->time_lag, bucket->time_lag + time_lag_size);
        for (uint32_t i = 0; i < time_lag_size; i++)
        {
            sum_value += (uint64_t)bucket->time_lag[i];
        }
        if (0 != time_lag_size)
        {
            lat_data->mean += (uint32_t)(sum_value / time_lag_size);
            lat_data->median += bucket->time_lag[time_lag_size / 2];
            lat_data->lower_quartile += bucket->time_lag[time_lag_size / 4];
            lat_data->upper_quartile +=
                bucket->time_lag[time_lag_size - time_lag_size / 4];
        }
    }
    // calculate acc seq data
    uint32_t bucket_size = lat_data->bucket_count;
    if (0 != bucket_size)
    {
        lat_data->mean /= bucket_size;
        lat_data->median /= bucket_size;
        lat_data->lower_quartile /= bucket_size;
        lat_data->upper_quartile /= bucket_size;
    }
    // calculate acc total seq data
    uint64_t total_count =
        lat_data->total_sample_count + (uint64_t)lat_data->sample_count;
    if (total_count >= OVERFLOW_THRESHOLD)
    {
        lat_data->overflow_warning = true;
        return;
    }
    if (lat_data->total_sample_count > 0)
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
    if ((lat_data->min != 0) &&
        ((lat_data->total_min == 0) || (lat_data->min < lat_data->total_min)))
    {
        lat_data->total_min = lat_data->min;
    }
}

void
process::LatencyProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccLatencyData* acc_lat_data =
        static_cast<lib::AccLatencyData*>(acc_data);

    uint32_t max_sid_size{lib::SID_SIZE};
    for (uint32_t sid = 0; sid < max_sid_size; sid++)
    {
        acc_lat_data->seq_data[sid].mean = 0;
        acc_lat_data->seq_data[sid].min = 0;
        acc_lat_data->seq_data[sid].max = 0;
        acc_lat_data->seq_data[sid].median = 0;
        acc_lat_data->seq_data[sid].lower_quartile = 0;
        acc_lat_data->seq_data[sid].upper_quartile = 0;
        acc_lat_data->seq_data[sid].sample_count = 0;
        acc_lat_data->seq_data[sid].bucket_count = 0;
        acc_lat_data->seq_data[sid].bucket.clear();

        if ((true == acc_lat_data->seq_data[sid].overflow_warning) ||
            (true == acc_data->need_erase))
        {
            acc_lat_data->seq_data[sid].total_mean = 0;
            acc_lat_data->seq_data[sid].total_min = 0;
            acc_lat_data->seq_data[sid].total_max = 0;
            acc_lat_data->seq_data[sid].total_median = 0;
            acc_lat_data->seq_data[sid].total_lower_quartile = 0;
            acc_lat_data->seq_data[sid].total_upper_quartile = 0;
            acc_lat_data->seq_data[sid].total_sample_count = 0;
            acc_lat_data->seq_data[sid].overflow_warning = 0;
        }
    }
    acc_data->need_erase = false;
}

bool
process::QueueProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);
    lib::AccQueueData* Acc_queue_data =
        static_cast<lib::AccQueueData*>(acc_data);

    if (air_queue_data->num_req > 0)
    {
        air_queue_data->depth_period_avg =
            (float)air_queue_data->sum_depth / air_queue_data->num_req;
    }
    if (Acc_queue_data->depth_total_max < air_queue_data->depth_period_max)
    {
        Acc_queue_data->depth_total_max = air_queue_data->depth_period_max;
    }

    double total_sum_depth_avg =
        Acc_queue_data->time_spent * Acc_queue_data->depth_total_avg;
    Acc_queue_data->time_spent += time;

    if (Acc_queue_data->time_spent > 0)
    {
        Acc_queue_data->depth_total_avg =
            (total_sum_depth_avg + (double)air_queue_data->depth_period_avg) /
            Acc_queue_data->time_spent;
    }

    return true;
}

void
process::QueueProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);
    lib::AccQueueData* acc_queue_data =
        static_cast<lib::AccQueueData*>(acc_data);

    air_queue_data->q_size = 0;
    air_queue_data->num_req = 0;
    air_queue_data->sum_depth = 0;
    air_queue_data->depth_period_avg = 0.0;
    air_queue_data->depth_period_max = 0;
    air_queue_data->dummy_u32 = 0;
    air_queue_data->access = 0;

    if (1 == acc_queue_data->need_erase)
    {
        acc_queue_data->depth_total_max = 0;
        acc_queue_data->time_spent = 0;
        acc_queue_data->depth_total_avg = 0.0;
        acc_queue_data->need_erase = 0;
    }
}
