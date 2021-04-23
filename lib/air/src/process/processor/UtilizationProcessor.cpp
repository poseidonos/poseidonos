
#include "src/process/processor/UtilizationProcessor.h"

bool
process::UtilizationProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::UtilizationData* air_util_data = static_cast<lib::UtilizationData*>(air_data);
    lib::AccUtilizationData* acc_util_data =
        static_cast<lib::AccUtilizationData*>(acc_data);

    acc_util_data->total_sum += air_util_data->sum;

    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
    {
        if (0 != air_util_data->sum)
        {
            air_util_data->percent[idx] =
                ((double)air_util_data->usage[idx] / air_util_data->sum) * 100;
        }

        acc_util_data->total_usage[idx] += air_util_data->usage[idx];
        if (0 != acc_util_data->total_sum)
        {
            acc_util_data->total_percent[idx] =
                ((double)acc_util_data->total_usage[idx] / acc_util_data->total_sum) * 100;
        }
    }

    return true;
}

void
process::UtilizationProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::UtilizationData* air_util_data = static_cast<lib::UtilizationData*>(air_data);
    lib::AccUtilizationData* acc_util_data =
        static_cast<lib::AccUtilizationData*>(acc_data);

    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
    {
        air_util_data->usage[idx] = 0;
        air_util_data->percent[idx] = 0.0;

        if (0 != acc_util_data->need_erase)
        {
            acc_util_data->total_usage[idx] = 0;
            acc_util_data->total_percent[idx] = 0.0;
        }
    }

    air_util_data->sum = 0;
    air_util_data->access = 0;

    if (0 != acc_util_data->need_erase)
    {
        acc_util_data->total_sum = 0;
        acc_util_data->need_erase = 0;
    }
}
