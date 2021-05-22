
#include "dummy_io.h"
#include "Air.h"

#include <pthread.h>
#include <thread>

#include <string>

bool DummyIO::run = true;

unsigned int DummyIO::read_iops = 0;
std::mutex DummyIO::iops_lock;

std::queue<int> DummyIO::sq[2];
std::mutex DummyIO::sq_lock[2];

std::queue<int> DummyIO::cq[2];
std::mutex DummyIO::cq_lock[2];

void
DummyIO::Run()
{
    run = true;
}

void
DummyIO::Stop()
{
    run = false;
}

void
DummyIO::Print()
{
    // core affinity 2
    cpu_set_t cpu {0};
    CPU_SET(2, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);
    pthread_setname_np(pthread_self(), "Print");

    while (run) {
        sleep(1);
        printf(" [DummyIO] Read_IOPS: %8u\r", read_iops);
        fflush(stdout);
        std::lock_guard<std::mutex> guard(iops_lock);
        read_iops = 0;
    }
}

void
DummyIO::SubmitIO()
{
    // core affinity 3
    cpu_set_t cpu {0};
    CPU_SET(3, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);
    pthread_setname_np(pthread_self(), "SubmitIO");

    int i {0};
    bool sq_push {false};

    while (run) {
        if (1000000000 < i)
            i = 0;

        {
            airlog("LAT_SUBMIT", "AIR_0", 0, i);
            airlog("LAT_IO_PATH", "AIR_0", 0, i);

            std::lock_guard<std::mutex> guard(sq_lock[0]);
            if (sq[0].size() < 256) {
                sq[0].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push) {
                airlog("LAT_IO_PATH", "AIR_1", 0, i);
                airlog("LAT_SUBMIT", "AIR_1", 0, i);
                i++;
            }
            airlog("UTIL_SUBMIT_THR", "AIR_SUBMIT", 0, 10);
        }

        {
            airlog("LAT_IO_PATH", "AIR_0", 1, i);

            std::lock_guard<std::mutex> guard(sq_lock[1]);
            if (sq[1].size() < 256) {
                sq[1].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push) {
                airlog("LAT_IO_PATH", "AIR_1", 1, i);
                i++;
            }
            airlog("UTIL_SUBMIT_THR", "AIR_SUBMIT", 1, 10);
        }

    }
}

void
DummyIO::ProcessIO(int qid)
{
    // core affinity 4, 5
    cpu_set_t cpu {0};
    CPU_SET(4 + qid, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);
    pthread_setname_np(pthread_self(), "ProcessIO");

    while (run) {
        bool valid {false};
        int value {0};

        {
            std::lock_guard<std::mutex> guard(sq_lock[qid]);
            if (!sq[qid].empty()) {
                airlog("Q_SUBMISSION", "AIR_BASE", qid, sq[qid].size());
                valid = true;
                value = sq[qid].front();
                sq[qid].pop();

                airlog("LAT_PROCESS", "AIR_0", qid, value);
                airlog("LAT_IO_PATH", "AIR_2", qid, value);
                airlog("UTIL_SUBMIT_THR", "AIR_PROCESS", qid, 10);
            }
        }

        if (valid) {
            std::lock_guard<std::mutex> guard(cq_lock[qid]);
            if (cq[qid].size() < 256) {
                cq[qid].push(value);
                airlog("LAT_PROCESS", "AIR_1", qid, value);
            }
        }
    }
}

void
DummyIO::CompleteIO()
{
    // core affinity 6
    cpu_set_t cpu {0};
    CPU_SET(6, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);
    pthread_setname_np(pthread_self(), "CompleteIO");

    while (run) {

        int count {0};
        int value {0};

        {
            std::lock_guard<std::mutex> guard(cq_lock[0]);
            if (!cq[0].empty()) {
                airlog("Q_COMPLETION", "AIR_BASE", 0, cq[0].size());
                count++;
                value = cq[0].front();

                airlog("LAT_COMPLETE", "AIR_0", 0, value);

                cq[0].pop();

                airlog("PERF_BENCHMARK", "AIR_READ", 0, 4096);
                airlog("CNT_TEST_EVENT", "AIR_COMPLETE", 0, 1);
                airlog("LAT_IO_PATH", "AIR_3", 0, value);
                airlog("LAT_COMPLETE", "AIR_1", 0, value);
                airlog("UTIL_SUBMIT_THR", "AIR_COMPLETE", 0, 3);
            }
        }

        {
            std::lock_guard<std::mutex> guard(cq_lock[1]);
            if (!cq[1].empty()) {
                airlog("Q_COMPLETION", "AIR_BASE", 1, cq[1].size());
                count++;
                value = cq[1].front();
                cq[1].pop();
                
                airlog("PERF_BENCHMARK", "AIR_READ", 1, 4096);
                airlog("CNT_TEST_EVENT", "AIR_COMPLETE", 1, -1);
                
                airlog("LAT_IO_PATH", "AIR_3", 1, value);
                airlog("UTIL_SUBMIT_THR", "AIR_COMPLETE", 1, 10);
            }
        }

        if (count) {
            std::lock_guard<std::mutex> guard(iops_lock);
            read_iops += count;
        }

    }
}
