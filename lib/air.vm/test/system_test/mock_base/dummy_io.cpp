
#include "dummy_io.h"
#include "Air.h"

#include <pthread.h>
#include <thread>

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
            AIRLOG(LAT_SUBMIT, 0, 0, i);
            AIRLOG(LAT_IO_PATH, 0, 0, i);

            std::lock_guard<std::mutex> guard(sq_lock[0]);
            if (sq[0].size() < 256) {
                sq[0].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push) {
                AIRLOG(LAT_IO_PATH, 0, 1, i);
                AIRLOG(LAT_SUBMIT, 0, 1, i);
                i++;
            }
            AIRLOG(UTIL_SUBMIT_THR, 0, 0, 30);
        }

        {
            AIRLOG(LAT_IO_PATH, 0, 0, i);

            std::lock_guard<std::mutex> guard(sq_lock[1]);
            if (sq[1].size() < 256) {
                sq[1].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push) {
                AIRLOG(LAT_IO_PATH, 0, 1, i);
                i++;
            }
            AIRLOG(UTIL_SUBMIT_THR, 0, 1, 12);
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
                AIRLOG(Q_SUBMISSION, qid,
                        sq[qid].size(), sq[qid].size());
                valid = true;
                value = sq[qid].front();
                sq[qid].pop();

                AIRLOG(LAT_PROCESS, qid, 0, value);
                AIRLOG(LAT_IO_PATH, 0, 2, value);
            }
        }

        if (valid) {
            std::lock_guard<std::mutex> guard(cq_lock[qid]);
            if (cq[qid].size() < 256) {
                cq[qid].push(value);
                AIRLOG(LAT_PROCESS, qid, 1, value);
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
                AIRLOG(Q_COMPLETION, 0,
                        cq[0].size(), cq[0].size());
                count++;
                value = cq[0].front();

                AIRLOG(LAT_COMPLETE, 0, 0, value);

                cq[0].pop();

                AIRLOG(PERF_BENCHMARK, 0, AIR_READ, 4096);
                AIRLOG(CNT_TEST_EVENT, 0, 0, 1);
                AIRLOG(LAT_IO_PATH, 0, 3, value);
                AIRLOG(LAT_COMPLETE, 0, 1, value);
            }
        }

        {
            std::lock_guard<std::mutex> guard(cq_lock[1]);
            if (!cq[1].empty()) {
                AIRLOG(Q_COMPLETION, 1,
                        cq[1].size(), cq[1].size());
                count++;
                value = cq[1].front();
                cq[1].pop();
                AIRLOG(PERF_BENCHMARK, 1, AIR_READ, 4096);
                AIRLOG(CNT_TEST_EVENT, 1, 0, 1);
                
                AIRLOG(LAT_IO_PATH, 0, 3, value);
            }
        }

        if (count) {
            std::lock_guard<std::mutex> guard(iops_lock);
            read_iops += count;
        }

    }
}
