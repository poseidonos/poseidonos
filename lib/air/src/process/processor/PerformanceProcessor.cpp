
#include "src/process/processor/PerformanceProcessor.h"

bool
process::PerformanceProcessor::_ProcessData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(air_data);
    lib::AccPerformanceData* acc_perf_data =
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
        double common_divisor = (double)(acc_perf_data->time_spent + time);

        acc_perf_data->iops_read_avg =
            ((double)acc_perf_data->iops_read_avg / common_divisor *
                acc_perf_data->time_spent) +
            ((double)perf_data->iops_read / common_divisor * time);

        acc_perf_data->iops_write_avg =
            ((double)acc_perf_data->iops_write_avg / common_divisor *
                acc_perf_data->time_spent) +
            ((double)perf_data->iops_write / common_divisor * time);

        acc_perf_data->bandwidth_read_avg =
            ((double)acc_perf_data->bandwidth_read_avg / common_divisor *
                acc_perf_data->time_spent) +
            ((double)perf_data->bandwidth_read / common_divisor * time);

        acc_perf_data->bandwidth_write_avg =
            ((double)acc_perf_data->bandwidth_write_avg / common_divisor *
                acc_perf_data->time_spent) +
            ((double)perf_data->bandwidth_write / common_divisor * time);
    }
    acc_perf_data->time_spent += time;

    return true;
}

void
process::PerformanceProcessor::_InitData(lib::Data* air_data,
    lib::AccData* acc_data)
{
    lib::AccPerformanceData* acc_perf_data =
        static_cast<lib::AccPerformanceData*>(acc_data);
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(air_data);

    perf_data->bandwidth_read = 0;
    perf_data->bandwidth_write = 0;
    perf_data->bandwidth_total = 0;
    perf_data->iops_read = 0;
    perf_data->iops_write = 0;
    perf_data->iops_total = 0;
    perf_data->packet_cnt.clear();
    perf_data->access = 0;

    if (0 != acc_perf_data->need_erase)
    {
        acc_perf_data->bandwidth_read_avg = 0;
        acc_perf_data->bandwidth_write_avg = 0;
        acc_perf_data->iops_read_avg = 0;
        acc_perf_data->iops_write_avg = 0;
        acc_perf_data->time_spent = 0;
        acc_perf_data->need_erase = 0;
    }
}
