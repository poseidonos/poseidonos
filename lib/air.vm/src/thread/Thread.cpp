
#include "src/thread/Thread.h"

#include <sched.h>
#include <unistd.h>

void*
thread::OneTimeThread::RunThreadLoop(void* thread)
{
    OneTimeThread* p_thread = (OneTimeThread*)thread;

    if (pthread_setaffinity_np(pthread_self(), sizeof(p_thread->cpu_set),
            &(p_thread->cpu_set)) < 0)
    {
        // perror("pthread_setaffinity_np failed!!!");
    }

    while (p_thread->_IsRun())
    {
        if (p_thread->_TryLock() == 0)
        {
            p_thread->RunThread(p_thread->run_skip_count);
            p_thread->run_skip_count = 0;
            p_thread->_Unlock();
        }
        else
        {
            p_thread->run_skip_count++;
        }

        pthread_cond_wait(&g_thread_cond, &g_thread_mutex);
        p_thread->_Unlock();
    }

    return nullptr;
}

void
thread::OneTimeThread::StartThread(void)
{
    thread_run = true;
    pthread_create(&thread, nullptr, &thread::OneTimeThread::RunThreadLoop,
        (void*)this);
    pthread_detach(thread);
}

void
thread::OneTimeThread::PullTrigger(void)
{
    pthread_cond_signal(&g_thread_cond);
}

void*
thread::PeriodicThread::RunThreadLoop(void* thread)
{
    PeriodicThread* p_thread = (PeriodicThread*)thread;

    if (pthread_setaffinity_np(pthread_self(), sizeof(p_thread->cpu_set),
            &(p_thread->cpu_set)) < 0)
    {
        // perror("pthread_setaffinity_np failed!!!");
    }

    while (p_thread->_IsRun())
    {
        p_thread->RunThread(0);
        usleep(9905); // 10ms
    }

    return nullptr;
}

void
thread::PeriodicThread::StartThread(void)
{
    thread_run = true;
    pthread_create(&thread, nullptr, &thread::PeriodicThread::RunThreadLoop,
        (void*)this);
}
