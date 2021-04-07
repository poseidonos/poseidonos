#include <assert.h>

#include "src/scheduler/io_dispatcher.h"

namespace ibofos
{
IODispatcher::IODispatcher(void)
{

}

IODispatcher::~IODispatcher(void)
{
}

uint32_t
IODispatcher::_GetLogicalCore(cpu_set_t cpuSet)
{
    uint32_t logicalCore = 0;

    return logicalCore;
}

IOWorker *
IODispatcher::AddIOWorker(cpu_set_t cpuSet, bool kernelWorker)
{
    uint32_t logicalCore = 1;
    IOWorker *retIOWorker = new IOWorker(cpuSet, kernelWorker);
    ioWorkerMap.insert(IOWorkerPair(logicalCore, retIOWorker));

    return retIOWorker;
   //return nullptr;
}

uint32_t
IODispatcher::RemoveIOWorker(cpu_set_t cpuSet)
{
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return IOWorker according to device
 *
 * @Param    device
 *
 * @Returns  The pointer of proper IOWorker
 */
/* --------------------------------------------------------------------------*/
IOWorker*
IODispatcher::GetIOWorker(UBlockDevice* device)
{
    
    IOWorkerMapIter it;

    for (it = ioWorkerMap.begin(); it != ioWorkerMap.end(); it++)
    {
        if (it->second->HasDevice(device))
        {
            return it->second;
        }
    }
    
    return nullptr;
}

}  // namespace ibofos

