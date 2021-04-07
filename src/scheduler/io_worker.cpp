/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "io_worker.h"

#include <unistd.h>

#include <iomanip>
#include <string>
#include <thread>

#include "spdk/thread.h"
#include "src/device/ublock_device.h"
#include "src/include/ibof_event_id.hpp"
#include "src/include/memory.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/io_queue.h"

#if defined QOS_ENABLED_BE
#include "spdk/event.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/scheduler/event.h"
#endif

namespace ibofos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor 
 *
 * @Param    coreAffinityInput
 */
/* --------------------------------------------------------------------------*/
IOWorker::IOWorker(cpu_set_t cpuSetInput, uint32_t id)
: cpuSet(cpuSetInput),
  ioQueue(new IOQueue),
  currentOutstandingIOCount(0),
  spdkThread(nullptr),
  exit(false),
  id(id)
{
    pthread_rwlock_init(&deviceListLock, nullptr);
    thread = new std::thread(&IOWorker::Run, this);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
IOWorker::~IOWorker(void)
{
    exit = true;
    thread->join();
    delete ioQueue;
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
IOWorker::EnqueueUbio(UbioSmartPtr ubio)
{
    ioQueue->EnqueueUbio(ubio);
}

IOWorker::DeviceOperation::DeviceOperation(IOWorker::DeviceOperationType type,
    UBlockDevice* device)
: command(type),
  device(device),
  done(false)
{
}

void
IOWorker::DeviceOperationQueue::SubmitAndWait(IOWorker::DeviceOperationType type,
    UBlockDevice* device)
{
    DeviceOperation operation(type, device);
    {
        std::unique_lock<std::mutex> lock(queueLock);
        queue.push(&operation);
    }
    operation.WaitDone();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Adds a UBlockDevice to be controlled by this IOWorker
 * @Param       device: a device to add
 * @return      device count currently added for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::AddDevice(UBlockDevice* device)
{
    if (device != nullptr)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOWORKER_DEVICE_ADDED;
        IBOF_TRACE_INFO(eventId, IbofEventId::GetString(eventId),
            device->GetName(), id);
        operationQueue.SubmitAndWait(INSERT, device);
    }
    else
    {
        IBOF_TRACE_WARN(5201, "Try to add null device to user IO worker");
    }

    return deviceList.size();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Adds UBlockDevices in the given list to be controlled by this IOWorker
 * @Param       inputList: a list of devices to add
 * @return      device count currently added for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::AddDevices(std::vector<UBlockDevice*>* inputList)
{
    uint32_t size = 0;
    for (auto& device : *inputList)
    {
        size = AddDevice(device);
    }

    return size;
}

void
IOWorker::DeviceOperation::WaitDone(void)
{
    while (done == false)
    {
        usleep(1);
    }
}

void
IOWorker::DeviceOperation::SetDone(void)
{
    done = true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Removes the given UBlockDevice from the list for this IOWorker
 * @Param       device: a UBlockDevice to remove from the list for this IOWorker
 * @return      device count currently left for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::RemoveDevice(UBlockDevice* device)
{
    if (HasDevice(device))
    {
        operationQueue.SubmitAndWait(REMOVE, device);
    }

    return deviceList.size();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Checks if the given UBlockDevice is in the list for this IOWorker
 * @Param       device: a UBlockDevice to find in the list for this IOWorker
 * @return      true if the given UBlockDevice is in the list, otherwise false.
 */
/* --------------------------------------------------------------------------*/
bool
IOWorker::HasDevice(UBlockDevice* device)
{
    pthread_rwlock_rdlock(&deviceListLock);
    DeviceSetIter it = deviceList.find(device);
    bool existance = (it != deviceList.end());
    pthread_rwlock_unlock(&deviceListLock);

    return existance;
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
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    std::ostringstream nameBucket;
    nameBucket << "UDDIOWorker" << id;
    std::string threadName = nameBucket.str();
    const char* cStringName = threadName.c_str();
    pthread_setname_np(pthread_self(), cStringName);

#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->EventParamterInit(id);
#endif

    UbioSmartPtr ubio = nullptr;
    while (false == exit)
    {
        ubio = ioQueue->DequeueUbio();
        while (nullptr != ubio)
        {
            _SubmitAsyncIO(ubio);
            _DoPeriodicJob();
            ubio = ioQueue->DequeueUbio();
#if defined QOS_ENABLED_BE
            currentOutstandingIOCount -= QosManagerSingleton::Instance()->EventQosPoller(id);
#endif
        }
        _DoPeriodicJob();
        usleep(1);
    }
}

void
IOWorker::_DoPeriodicJob(void)
{
    _CompleteCommand();
    _HandleDeviceOperation();
}

IOWorker::DeviceOperation*
IOWorker::DeviceOperationQueue::Pop(void)
{
    DeviceOperation* ret = nullptr;
    if (queueLock.try_lock())
    {
        if (queue.empty() == false)
        {
            ret = queue.front();
            queue.pop();
        }
        queueLock.unlock();
    }
    return ret;
}

IOWorker::DeviceOperationType
IOWorker::DeviceOperation::GetCommand(void)
{
    return command;
}

UBlockDevice*
IOWorker::DeviceOperation::GetDevice(void)
{
    return device;
}

void
IOWorker::_HandleDeviceOperation(void)
{
    DeviceOperation* operation = operationQueue.Pop();

    if (operation != nullptr)
    {
        try
        {
            UBlockDevice* device = operation->GetDevice();

            switch (operation->GetCommand())
            {
                case INSERT:
                {
                    pthread_rwlock_wrlock(&deviceListLock);
                    deviceList.insert(device);
                    pthread_rwlock_unlock(&deviceListLock);
                    device->Open();
                    break;
                }
                case REMOVE:
                {
                    pthread_rwlock_wrlock(&deviceListLock);
                    deviceList.erase(device);
                    pthread_rwlock_unlock(&deviceListLock);
                    currentOutstandingIOCount -= device->Close();
                    break;
                }
                default:
                {
                    throw operation;
                    break;
                }
            }
            operation->SetDone();
        }
        catch (DeviceOperation* operation)
        {
            IBOF_TRACE_ERROR((uint32_t)IBOF_EVENT_ID::IOWORKER_OPERATION_NOT_SUPPORTED,
                IbofEventId::GetString(IBOF_EVENT_ID::IOWORKER_OPERATION_NOT_SUPPORTED),
                operation->GetCommand());
        }
    }
}
#if defined QOS_ENABLED_BE
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Decrement the current outstanding io count
 *
 * @Param    count
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::DecreaseCurrentOutstandingIoCount(int count)
{
    currentOutstandingIOCount -= count;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Decrement the current outstanding io count
 *
 * @Param    count
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::GetWorkerId(void)
{
    return id;
}
#endif

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Submit bio to device without waiting.
 *
 * @Param    ubio
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::_SubmitAsyncIO(UbioSmartPtr ubio)
{
    currentOutstandingIOCount++;
#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->SubmitAsyncIO(this, ubio);
#else
    int completionCount = 0;
    completionCount = ubio->GetUBlock()->SubmitAsyncIO(ubio);
    currentOutstandingIOCount -= completionCount;
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Handle completion of command submitted via each driver
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::_CompleteCommand(void)
{
    if (0 < currentOutstandingIOCount)
    {
        for (auto& it : deviceList)
        {
            int eventCount = it->CompleteIOs();

            if (likely(currentOutstandingIOCount >=
                    static_cast<uint32_t>(eventCount)))
            {
                currentOutstandingIOCount -= eventCount;
            }
            else
            {
                IBOF_TRACE_ERROR((uint32_t)IBOF_EVENT_ID::IOWORKER_UNDERFLOW_HAPPENED,
                    IbofEventId::GetString(IBOF_EVENT_ID::IOWORKER_UNDERFLOW_HAPPENED),
                    currentOutstandingIOCount, eventCount);
                currentOutstandingIOCount = 0;
            }
        }
    }
}

} // namespace ibofos
