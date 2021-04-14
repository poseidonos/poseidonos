
#ifndef AIR_THREAD_MANAGER_H
#define AIR_THREAD_MANAGER_H

#include "src/chain/ChainManager.h"
#include "src/thread/Thread.h"

namespace thread
{
class ThreadManager : public PeriodicThread
{
public:
    explicit ThreadManager(chain::ChainManager* new_chain_manager)
    : chain_manager(new_chain_manager)
    {
    }
    virtual void RunThread(uint32_t run_skip_count);

private:
    chain::ChainManager* chain_manager{nullptr};
};

} // namespace thread

#endif // AIR_THREAD_MANAGER_H
