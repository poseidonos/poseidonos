
#ifndef DUMMY_IO_H
#define DUMMY_IO_H

#include "result_handler.h"

#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <mutex>
#include <random>
#include <queue>

struct DummyCMD
{
    uint32_t lba;
    uint16_t size;
    bool write;
};

class DummyIO
{
public:
    void Run(std::string filename);
    void Stop();
    void PrintIO();
    void SubmitIO();
    void ProcessIO(int qid);
    void CompleteIO();
    void SleepIO();

private:
    DummyCMD MakeDummyCMD();

    static ResultHandler result_hdr;

    static unsigned int lba;
    static unsigned int max_lba;

    static unsigned int chunk_size;
    static unsigned int sleep_cycle;
    static unsigned int mixed_ratio;
    static unsigned int token_read;
    static unsigned int token_write;

    static bool sequential_workload;
    static bool write_pattern;
    static bool run;

    static std::atomic<unsigned int> read_iops;
    static std::atomic<unsigned int> write_iops;
    static std::atomic<unsigned int> sleep_cnt;

    static std::string filename;

    static std::queue<DummyCMD> sq[2];
    static std::mutex sq_lock[2];

    static std::queue<DummyCMD> cq[2];
    static std::mutex cq_lock[2];
};

#endif // #define DUMMY_IO_H
