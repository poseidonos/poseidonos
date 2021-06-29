
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "Air_c.h"

static int run = 1;

void* test_func(void* data)
{
    int key = 0;

    while (run)
    {
        AIRLOG(PERF_TEST, AIR_READ, 0, 4096);
        AIRLOG(LAT_TEST, AIR_1, 0, key);
        usleep(100);
        AIRLOG(LAT_TEST, AIR_1, 1, key);
        key++;
        AIRLOG(Q_TEST, AIR_2, 2, 22);
        AIRLOG_FAKE(Q_TEST_OFF, AIR_3, 3, 33);
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
