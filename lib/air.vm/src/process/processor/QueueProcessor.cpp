
#include "src/process/processor/QueueProcessor.h"

bool
process::QueueProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::QueueData* air_queue_data = static_cast<lib::QueueData*>(air_data);
    lib::AccQueueData* acc_queue_data =
        static_cast<lib::AccQueueData*>(acc_data);

    if (air_queue_data->num_req > 0)
    {
        air_queue_data->depth_period_avg =
            (float)air_queue_data->sum_depth / air_queue_data->num_req;
    }
    if (acc_queue_data->depth_total_max < air_queue_data->depth_period_max)
    {
        acc_queue_data->depth_total_max = air_queue_data->depth_period_max;
    }

    double total_sum_depth_avg =
        acc_queue_data->time_spent * acc_queue_data->depth_total_avg;
    acc_queue_data->time_spent += time;

    if (0 != acc_queue_data->time_spent)
    {
        acc_queue_data->depth_total_avg =
            (total_sum_depth_avg + (double)air_queue_data->depth_period_avg) /
            acc_queue_data->time_spent;
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

    if (0 != acc_queue_data->need_erase)
    {
        acc_queue_data->depth_total_max = 0;
        acc_queue_data->time_spent = 0;
        acc_queue_data->depth_total_avg = 0.0;
        acc_queue_data->need_erase = 0;
    }
}
