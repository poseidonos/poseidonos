
#include "src/process/processor/LatencyProcessor.h"

#include <algorithm>
#include <cmath>

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
            (0 != acc_data->need_erase))
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
    acc_data->need_erase = 0;
}
