
#include "dummy_io.h"
#include "air.h"

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
    cpu_set_t cpu;
    CPU_SET(2, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    while (run) {
        sleep(1);
        std::lock_guard<std::mutex> guard(iops_lock);
        read_iops = 0;
    }
}

void
DummyIO::SubmitIO()
{
    // core affinity 3
    cpu_set_t cpu;
    CPU_SET(3, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    int i {0};
    bool sq_push {false};

    while (run) {
        if (1000000000 < i)
            i = 0;

        {
            std::lock_guard<std::mutex> guard(sq_lock[0]);
            if (sq[0].size() < 256) {
                sq[0].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push) {
                i++;
            }
        }

        {
            std::lock_guard<std::mutex> guard(sq_lock[1]);
            if (sq[1].size() < 256) {
                sq[1].push(i);
                sq_push = true;
            }
            else {
                sq_push = false;
            }

            if (sq_push)
                i++;
        }

        usleep(1);
    }
}

void
DummyIO::ProcessIO(int qid)
{
    // core affinity 4, 5
    cpu_set_t cpu;
    CPU_SET(4 + qid, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    while (run) {
        bool valid {false};
        int value {0};

        {
            std::lock_guard<std::mutex> guard(sq_lock[qid]);
            if (!sq[qid].empty()) {
                valid = true;
                value = sq[qid].front();
                sq[qid].pop();
            }
        }

        if (valid) {
            std::lock_guard<std::mutex> guard(cq_lock[qid]);
            if (cq[qid].size() < 256) {
                cq[qid].push(value);
            }
        }
        usleep(1);
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
        int count {0};
        int value {0};

        {
            std::lock_guard<std::mutex> guard(cq_lock[0]);
            if (!cq[0].empty()) {
                count++;
                value = cq[0].front();
                cq[0].pop();
            }
        }

        {
            std::lock_guard<std::mutex> guard(cq_lock[1]);
            if (!cq[1].empty()) {
                count++;
                cq[1].pop();
            }
        }
        value++;

        if (count) {
            std::lock_guard<std::mutex> guard(iops_lock);
            read_iops += count;
        }

        usleep(1);
    }
}
