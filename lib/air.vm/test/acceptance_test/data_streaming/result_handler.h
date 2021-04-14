
#ifndef RESULT_HANDLER_H
#define RESULT_HANDLER_H

#include <stdint.h>
#include <string>

class ResultHandler
{
public:
    void HandleData(unsigned int read_iops, unsigned int write_iops,
            unsigned int sleep_cnt, unsigned int tick_1ms_cnt); 
    bool IsUpdate(void);
    void OutputResult(std::string filename);
private:
    bool CheckAIRDataFormat(void);
    bool CheckAIRDataPeriod(unsigned int tick_1ms_cnt);

    uint32_t dummy_read_iops {0};
    uint32_t dummy_write_iops {0};
    uint32_t dummy_lat_avg {0};

    uint64_t dummy_total_sleep_cnt {0};
    uint64_t dummy_total_time {0};

    uint32_t air_read_iops {0};
    uint32_t air_write_iops {0};
    uint32_t air_lat_avg {0};

    uint32_t format_pass_count {0};
    uint32_t format_fail_count {0};

    uint32_t period_pass_count {0};
    uint32_t period_fail_count {0};

    off_t prev_file_size {0};
};
#endif // #define RESULT_HANDLER_H
