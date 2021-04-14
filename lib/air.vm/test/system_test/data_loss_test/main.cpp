
#include "Air.h"

#include <unistd.h>
#include <thread>

void LogPerf(int thr_num);
bool run {true};

int main(int argc, char *argv[])
{
    if (2 != argc)
    {
        printf("Usage: %s [thread num]\n", argv[0]);
        return -1;
    }

    AIR_INITIALIZE(0);
    AIR_ACTIVATE();

    int num_thr = atoi(argv[1]);
    int time {15};

    std::thread *thr = new std::thread[num_thr];
    for (int i = 0 ; i < num_thr ; i++)
    {
        thr[i] = std::thread(LogPerf, i);
    }

    sleep(time);
    run = false;

    for (int i = 0 ; i < num_thr ; i++)
    {
        thr[i].join();
    }
    
    sleep(5);

    AIR_DEACTIVATE();
    AIR_FINALIZE();

    return 0;
}

void LogPerf(int thr_num)
{
    cpu_set_t cpu;
    CPU_SET(1 + thr_num, &cpu);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu), &cpu);

    uint64_t log_cnt {0};
    sleep(5); // ramp up for detector to detect threads

    while (run)
    {
        log_cnt++;
        AIRLOG(PERF_TEST, 0, AIR_WRITE, 4096);
    }
    printf("%ld\n", log_cnt);
}
