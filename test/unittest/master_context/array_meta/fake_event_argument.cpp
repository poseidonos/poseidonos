#include <assert.h>

#include "src/scheduler/event_argument.h"

namespace ibofos
{

EventScheduler* EventArgument::eventScheduler = nullptr;
IODispatcher* EventArgument::ioDispatcher = nullptr;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor
 *           maxArgumentCount is needed for avoiding run time memory allocation
 *
 * @Param    maxArgumentCountInput
 */
/* --------------------------------------------------------------------------*/
EventArgument::EventArgument(unsigned int maxArgumentCountInput)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
EventArgument::~EventArgument(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Call only once
 *           Set static member variables for commonly sharing among event handlers
 *
 * @Param    eventSchedulerInput
 * @Param    ioDispatcherInput
 */
/* --------------------------------------------------------------------------*/
void
EventArgument::SetStaticMember(EventScheduler* eventSchedulerInput,
        IODispatcher* ioDispatcherInput)
{
    ioDispatcher = ioDispatcherInput;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Set argument to specified index
 *
 * @Param    argumentIndex
 * @Param    argumentInput
 */
/* --------------------------------------------------------------------------*/
void
EventArgument::AddArgument(unsigned int argumentIndex, void* argumentInput)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return argument from the internal array of specified index
 *
 * @Param    argumentIndex
 *
 * @Returns  The pointer of argument
 */
/* --------------------------------------------------------------------------*/
void*
EventArgument::GetArgument(unsigned int argumentIndex)
{
    return nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return maximum argument count
 *
 * @Returns  The maximum argument count
 */
/* --------------------------------------------------------------------------*/
unsigned int
EventArgument::GetArgumentCount(void)
{
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return EventScheduler
 *
 * @Returns  EventScheduler
 */
/* --------------------------------------------------------------------------*/
EventScheduler*
EventArgument::GetEventScheduler(void)
{

    return nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return IODispatcher
 *
 * @Returns  IODispatcher
 */
/* --------------------------------------------------------------------------*/
IODispatcher*
EventArgument::GetIODispatcher(void)
{
    return ioDispatcher;
}

void
EventArgument::AddIntArgument(unsigned int argumentIndex, uint64_t argumentInput)
{
}

}
