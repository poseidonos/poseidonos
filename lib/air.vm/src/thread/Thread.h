
#ifndef AIR_THREAD_H
#define AIR_THREAD_H

#include <pthread.h>

#include <cstdint>

namespace thread
{
static pthread_mutex_t g_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_thread_cond = PTHREAD_COND_INITIALIZER;

class AirThread
{
public:
    AirThread(void)
    {
    }
    virtual ~AirThread(void)
    {
        if (_IsRun())
        {
            JoinThread();
        }
    }
    virtual void RunThread(uint32_t run_skip_count) = 0;
    virtual void StartThread(void) = 0;
    static void*
    RunThreadLoop(void*)
    {
        return nullptr;
    }
    void
    JoinThread(void)
    {
        thread_run = false;
        pthread_cond_signal(&g_thread_cond);
        pthread_join(thread, nullptr);
    }
    void
    SetCpuSet(uint32_t cpu_num)
    {
        CPU_ZERO(&cpu_set);
        CPU_SET(cpu_num, &cpu_set);
    }
    uint32_t
    GetRunSkipCount(void) const
    {
        return run_skip_count;
    }

protected:
    int
    _TryLock(void)
    {
        return pthread_mutex_trylock(&g_thread_mutex);
    }
    int
    _Unlock(void)
    {
        return pthread_mutex_unlock(&g_thread_mutex);
    }
    bool
    _IsRun(void)
    {
        return thread_run;
    }

    pthread_t thread{0};
    cpu_set_t cpu_set{0};
    bool thread_run{false};
    uint32_t run_skip_count{0};
};

class OneTimeThread : public AirThread
{
public:
    static void* RunThreadLoop(void*);
    virtual void StartThread(void);
    virtual void PullTrigger(void);
    virtual void
    RunThread(uint32_t run_skip_count)
    {
        return;
    }
};

class PeriodicThread : public AirThread
{
public:
    static void* RunThreadLoop(void*);
    virtual void StartThread(void);
    virtual void
    RunThread(uint32_t run_skip_count)
    {
        return;
    }
};

} // namespace thread

#endif // AIR_THREAD_H
