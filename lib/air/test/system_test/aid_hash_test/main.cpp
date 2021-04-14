
#include "Air.h"

#include <unistd.h>
#include <thread>

void LogPerf(int thr_num);

#define AID_NUM 50
bool run {true};
char *aid_list_1[AID_NUM];

int main(int argc, char *argv[])
{
    int array_size = 1;
    if (argc == 2)
        array_size = atoi(argv[1]);
    for (int i = 0 ; i < AID_NUM ; i++) 
    {
        aid_list_1[i] = new char[array_size];
    }
 
    AIR_INITIALIZE(0);
    AIR_ACTIVATE();

    int num_thr = 1;
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
    
    sleep(2);

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
    int count {0};
    sleep(2); // ramp up for detector to detect threads

    while (run)
    {
        if (count == 100)
        {
            int idx = log_cnt % AID_NUM;
            log_cnt += 1;
            AIRLOG(PERF_TEST_1, (int64_t)aid_list_1[idx], AIR_WRITE, 4096);
            count = 0;
        }
        count++;
    }
    printf("Log Count: %ld\n", log_cnt);
}
