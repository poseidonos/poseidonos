
#include "src/process/processor/CountProcessor.h"

bool
process::CountProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::CountData* air_count_data = static_cast<lib::CountData*>(air_data);
    lib::AccCountData* acc_count_data =
        static_cast<lib::AccCountData*>(acc_data);

    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
    {
        acc_count_data->total_count[idx] += air_count_data->count[idx];
        acc_count_data->total_num_req[idx] += air_count_data->num_req[idx];

        if (0 != acc_count_data->total_num_req[idx])
        {
            acc_count_data->total_count_avg[idx] =
                (double)acc_count_data->total_count[idx] /
                acc_count_data->total_num_req[idx];
        }
    }

    return true;
}

void
process::CountProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccCountData* acc_count_data =
        static_cast<lib::AccCountData*>(acc_data);
    lib::CountData* air_count_data = static_cast<lib::CountData*>(air_data);

    for (uint64_t idx = 0; idx < lib::ENUM_SIZE; idx++)
    {
        air_count_data->count[idx] = 0;
        air_count_data->num_req[idx] = 0;

        if (0 != acc_count_data->need_erase)
        {
            acc_count_data->total_count[idx] = 0;
            acc_count_data->total_count_avg[idx] = 0.0;
            acc_count_data->total_num_req[idx] = 0;
        }
    }

    air_count_data->access = 0;

    if (0 != acc_count_data->need_erase)
    {
        acc_count_data->need_erase = 0;
    }
}
