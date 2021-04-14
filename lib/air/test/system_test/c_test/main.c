
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "Air.h"

static int run = 1;

void* test_func(void* data)
{
    int key = 0;

    while (run)
    {
        AIRLOG(PERF_TEST, 0, AIR_READ, 4096);
        AIRLOG(PERF_BUILD_FALSE, 1, AIR_READ, 4096);
        AIRLOG(LAT_TEST, 2, 0, key);
        usleep(100);
        AIRLOG(LAT_TEST, 2, 1, key);
        key++;
        AIRLOG(Q_TEST, 3, 3, 33);
        AIRLOG(Q_RUN_OFF, 4, 4, 44);
    }
}

int main(void)
{
    pthread_t thread;
    int thr_id;

    AIR_INITIALIZE(0);
    AIR_ACTIVATE();

    thr_id = pthread_create(&thread, NULL, test_func, NULL);

    sleep(100);
    run = 0;

    if (0 <= thr_id)
    {
        pthread_join(thread, NULL);
    }

    AIR_DEACTIVATE();
    AIR_FINALIZE();

    return 0;
}
