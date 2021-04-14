
#include "src/thread/ThreadManager.h"

void
thread::ThreadManager::RunThread(uint32_t run_skip_count)
{
    chain_manager->PullTrigger();
}
