
#ifndef RULE_CHECKER_H
#define RULE_CHECKER_H

#include <string>

class RuleChecker
{
public:
    bool CheckRule(int argc, char **argv);
    unsigned int GetChunkSize() { return chunk_size; }
    unsigned int GetSleepCycle() { return sleep_cycle; }
    unsigned int GetRunTime() { return run_time; }
    unsigned int GetMixedRatio() { return mixed_ratio; }
    bool IsSequentialWorkload() { return sequential_workload; }
    bool IsWritePattern() { return write_pattern; }
    unsigned int GetErrorMargin() { return error_margin; }
    unsigned int GetConfidenceInterval() { return confidence_interval; }
    std::string GetFilename() { return filename; }

private:
    void PrintHelp();
    bool CheckSize(char *arg);
    bool CheckWorkload(char *arg);
    bool CheckSleep(char *arg);
    bool CheckRuntime(char *arg);
    bool CheckErrorMargin(char *arg);
    bool CheckConfidenceInterval(char *arg);
    bool CheckFilename(char *arg);

    unsigned int chunk_size {4096};     // granularity: byte
    unsigned int sleep_cycle {1000};    // resolution: nano-second
    unsigned int run_time {60};         // resolution: second
    unsigned int mixed_ratio {0};
    bool sequential_workload {false};
    bool write_pattern {false};
    unsigned int error_margin {3};
    unsigned int confidence_interval {0};
    std::string filename;
};

#endif // #define RULE_CHECKER_H
