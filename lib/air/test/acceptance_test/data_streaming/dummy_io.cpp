
#include "dummy_io.h"
#include "air.h"

#include <pthread.h>
#include <thread>

using namespace air;

ResultHandler DummyIO::result_hdr;

unsigned int DummyIO::lba = 0;
unsigned int DummyIO::max_lba = 1000000;

unsigned int DummyIO::chunk_size = 4096;
unsigned int DummyIO::sleep_cycle = 1000000; // 1ms
unsigned int DummyIO::mixed_ratio = 30;
unsigned int DummyIO::token_read = 0;
unsigned int DummyIO::token_write = 0;

bool DummyIO::sequential_workload = false;
bool DummyIO::write_pattern = true;
bool DummyIO::run = true;

std::atomic<unsigned int> DummyIO::read_iops;
std::atomic<unsigned int> DummyIO::write_iops;
std::atomic<unsigned int> DummyIO::sleep_cnt;

std::string DummyIO::filename;

std::queue<DummyCMD> DummyIO::sq[2];
std::mutex DummyIO::sq_lock[2];

std::queue<DummyCMD> DummyIO::cq[2];
std::mutex DummyIO::cq_lock[2];

static std::random_device rd;
static std::mt19937_64 gen(rd());
static std::uniform_int_distribution<unsigned int> dist(1, 1000000);

void
DummyIO::Run(std::string new_filename)
{
    printf(" [DummyIO] start with %s\n", new_filename.c_str());

    filename = new_filename;

    run = true;
}

void
DummyIO::Stop()
{
    run = false;
}

void
DummyIO::PrintIO()
{
    // core affinity 2
    cpu_set_t cpu;
    CPU_SET(2, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    unsigned int tmp_read_iops, tmp_write_iops, tmp_sleep_cnt;
    int ramp_up_cnt {1};
    unsigned int tick_1ms_cnt {0};

    while (run) {
        usleep(1000); // 1ms
        tick_1ms_cnt++;

        if (result_hdr.IsUpdate())
        {
            tmp_read_iops = read_iops;
            read_iops = 0;
            tmp_write_iops = write_iops;
            write_iops = 0;
            tmp_sleep_cnt = sleep_cnt;
            sleep_cnt = 0;

            if (ramp_up_cnt > 0)
            {
                ramp_up_cnt--;
                continue;
            }

            result_hdr.HandleData(tmp_read_iops, tmp_write_iops,
                    tmp_sleep_cnt, tick_1ms_cnt);

            tick_1ms_cnt = 0;
        }
    }

    if (!filename.empty())
    {
        result_hdr.OutputResult(filename);
    }
}

DummyCMD
DummyIO::MakeDummyCMD()
{
    DummyCMD cmd;

    if (mixed_ratio)
    {
        if (token_read)
        {
            cmd.write = false;
            token_read--;
        }
        else if (token_write)
        {
            cmd.write = true;
            token_write--;
        }
        else
        {
            token_write = mixed_ratio;
            token_read = 100 - mixed_ratio;

            cmd.write = false;
            token_read--;
        }
    }
    else
    {
        if (write_pattern)
        {
            cmd.write = true;
        }
        else
        {
            cmd.write = false;
        }
    }

    cmd.size = (uint16_t)chunk_size;

    if (sequential_workload)
    {
        cmd.lba = lba++;
    }
    else
    {
        cmd.lba = dist(gen);
    }

    return cmd;
}

void
DummyIO::SubmitIO()
{
    // core affinity 3
    cpu_set_t cpu;
    CPU_SET(3, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    while (run) {

        DummyCMD cmd = MakeDummyCMD();
        {
            std::lock_guard<std::mutex> guard(sq_lock[0]);
            if (sq[0].size() < 256) {
                sq[0].push(cmd);
            }
        }

        cmd = MakeDummyCMD();
        {
            std::lock_guard<std::mutex> guard(sq_lock[1]);
            if (sq[1].size() < 256) {
                sq[1].push(cmd);
            }
        }

    }
}

void
DummyIO::ProcessIO(int qid)
{
    // core affinity 4, 5
    cpu_set_t cpu;
    CPU_SET(4 + qid, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    bool valid {false};
    DummyCMD cmd;

    while (run) {

        if (!valid)
        {
            std::lock_guard<std::mutex> guard(sq_lock[qid]);
            if (!sq[qid].empty()) {
                valid = true;
                cmd = sq[qid].front();
                sq[qid].pop();
            }
        }

        if (valid) {
            std::lock_guard<std::mutex> guard(cq_lock[qid]);
            if (cq[qid].size() < 256) {
                cq[qid].push(cmd);
                valid = false;
            }
        }
    }
}

void
DummyIO::CompleteIO()
{
    // core affinity 6
    cpu_set_t cpu;
    CPU_SET(6, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    while (run) {

        DummyCMD cmd;

        {
            std::lock_guard<std::mutex> guard(cq_lock[0]);
            if (!cq[0].empty()) {
                cmd = cq[0].front();
                cq[0].pop();

                if (true == cmd.write)
                {
                    AIRLOG_PERF(PERF_TEST, 0, IO_WRITE, cmd.size);
                    write_iops++;
                }
                else
                {
                    AIRLOG_PERF(PERF_TEST, 0, IO_READ, cmd.size);
                    read_iops++;
                }
            }
        }

        {
            std::lock_guard<std::mutex> guard(cq_lock[1]);
            if (!cq[1].empty()) {
                cmd = cq[1].front();
                cq[1].pop();

                if (true == cmd.write)
                {
                    AIRLOG_PERF(PERF_TEST, 1, IO_WRITE, cmd.size);
                    write_iops++;
                }
                else
                {
                    AIRLOG_PERF(PERF_TEST, 1, IO_READ, cmd.size);
                    read_iops++;
                }
            }
        }
    }
}

void
DummyIO::SleepIO()
{
    // core affinity 7
    cpu_set_t cpu;
    CPU_SET(7, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    unsigned int key {0};
    while (run) {
        AIRLOG_MULTI_LAT(LAT_TEST, 0, 0, key);
        if (sleep_cycle >= 1000)
        {
            usleep(sleep_cycle/1000);
        }
        else
        {
            volatile unsigned int loop_cnt {30};
            while (loop_cnt)
            {
                loop_cnt--;
            }
        }
        AIRLOG_MULTI_LAT(LAT_TEST, 0, 1, key);

        sleep_cnt++;
        key++;
    }
}

