#include <unistd.h>
#include <thread>

#include "src/include/memory.h"
#include "src/device/mock_device.h"
#include "src/scheduler/io_worker.h"

//#include "tool/air/air.h"

namespace ibofos
{

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor 
 *
 * @Param    coreAffinityInput
 */
/* --------------------------------------------------------------------------*/
IOWorker::IOWorker(cpu_set_t cpuSetInput, bool kernelWorker)
{
    thread = new std::thread(&IOWorker::Run, this);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
IOWorker::~IOWorker(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Push bio for IOWorker
 *           MPSC queue (handlers -> IOWorker)
 *
 * @Param    pobjInput
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::EnqueueUbio(Ubio* ubio)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Submit bio to device and wait until done
 *
 * @Param    ubio
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::SyncIO(Ubio* ubio)
{
    ubio->dev->SyncIO(ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Submit bio to device and wait until done
 *
 * @Param    direction
 * @Param    buffer
 * @Param    address
 * @Param    size
 * @Param    device
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::SyncIO(UbioDir direction,
                 void* buffer,
                 uint64_t address,
                 uint64_t size,
                 UBlockDevice* device)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Adds a UBlockDevice to be controlled by this IOWorker
 * @Param       device: a device to add
 * @return      device count currently added for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::AddDevice(UBlockDevice *device)
{
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Adds UBlockDevices in the given list to be controlled by this IOWorker
 * @Param       inputList: a list of devices to add
 * @return      device count currently added for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::AddDevices(std::vector<UBlockDevice *> *inputList)
{
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Removes the given UBlockDevice from the list for this IOWorker
 * @Param       device: a UBlockDevice to remove from the list for this IOWorker
 * @return      device count currently left for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::RemoveDevice(UBlockDevice *device)
{
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Checks if the given UBlockDevice is in the list for this IOWorker
 * @Param       device: a UBlockDevice to find in the list for this IOWorker
 * @return      true if the given UBlockDevice is in the list, otherwise false.
 */
/* --------------------------------------------------------------------------*/
bool
IOWorker::HasDevice(UBlockDevice *device)
{
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Big loop of IOWorker
 *           Get bio from IOQueue and submit to device
 *           Only libaio is supported now
 */
/* --------------------------------------------------------------------------*/

void
IOWorker::Run(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Pop a bio from IOWorker
 *
 * @Param    None
 * @return   Ubio *
 */
/* --------------------------------------------------------------------------*/
Ubio *
IOWorker::_DequeueUbio(void)
{
    return nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Submit bio to device without waiting.
 *
 * @Param    ubio
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::_AsyncIO(Ubio *ubio)
{
}

void
IOWorker::_BulkAsyncIO(std::vector<Ubio*>& ubioArray, uint32_t ubioCount)
{
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Handle completion of command submitted via each driver
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::_HandleCompletion(void)
{
}

} // namespace ibofos

