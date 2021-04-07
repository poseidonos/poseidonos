#include <assert.h>

#include "src/io/ubio.h"
#include "src/scheduler/io_queue.h"

namespace ibofos
{
IOQueue::IOQueue(void)
: lastQueue(queueMap.end())
{
}

IOQueue::~IOQueue(void)
{
}

Ubio*
IOQueue::DequeueUbio(void)
{
    Ubio* ubio = nullptr;

    return ubio;
}

UbioArray*
IOQueue::DequeueBulkUbio(void)
{
    return nullptr;
}

IOQueue::QueueMapIter
IOQueue::_GetNextIter(QueueMapIter& targetIter)
{
    QueueMapIter nextIter = targetIter;

    return nextIter;
}

void
IOQueue::EnqueueUbioWithClassification(Ubio* input)
{
}


void
IOQueue::EnqueueUbio(Ubio* input)
{
}

int
IOQueue::GetQueueSize(void){
    return 0;
}

} // namespace ibofos
