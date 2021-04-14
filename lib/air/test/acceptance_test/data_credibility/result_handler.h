
#ifndef RESULT_HANDLER_H
#define RESULT_HANDLER_H

#include <stdint.h>
#include <string>

class ResultHandler
{
public:
    bool HandleData(unsigned int read_iops, unsigned int write_iops,
            unsigned int sleep_cnt, unsigned int error_margin,
            unsigned int confidence_interval);
    void OutputResult(std::string filename, bool result);
private:
    bool GetAIRData();
    void ComparePerfData();
    void CompareLatData();
    bool CheckResult();

    uint32_t dummy_read_iops {0};
    uint32_t dummy_write_iops {0};
    uint32_t dummy_lat_avg {0};
    uint64_t dummy_total_sleep_cnt {0};
    uint64_t dummy_total_time {0};

    uint32_t air_read_iops {0};
    uint32_t air_write_iops {0};
    uint32_t air_lat_avg {0};

    uint32_t iops_pass_cnt {0};
    uint32_t iops_fail_cnt {0};
    uint32_t lat_pass_cnt {0};
    uint32_t lat_fail_cnt {0};

    uint32_t error_margin {3};
    uint32_t confidence_interval {0};

    uint32_t fail_count {0};
    bool iops_pass {false};
    bool lat_pass {false};
};
#endif // #define RESULT_HANDLER_H
