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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include "io_worker_submission_notifier.h"
#include "spdk/event.h"
#include "spdk/thread.h"
#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/device/base/ublock_device_submission_adapter.h"
#include "src/device/device_detach_trigger.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/event_scheduler/event.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/io_queue.h"
#include "src/logger/logger.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor 
 *
 * @Param    coreAffinityInput
 */
/* --------------------------------------------------------------------------*/
IOWorker::IOWorker(cpu_set_t cpuSetInput, uint32_t id,
    DeviceDetachTrigger* detachTriggerArg, QosManager* qosManagerArg)
: cpuSet(cpuSetInput),
  ioQueue(new IOQueue),
  currentOutstandingIOCount(0),
  exit(false),
  id(id),
  detachTrigger(detachTriggerArg),
  qosManager(qosManagerArg)
{
    if (nullptr == detachTrigger)
    {
        detachTrigger = new DeviceDetachTrigger();
        productDetachTrigger = true;
    }
    if (nullptr == qosManager)
    {
        qosManager = QosManagerSingleton::Instance();
    }
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

    if (true == productDetachTrigger)
    {
        delete detachTrigger;
    }
}

uint32_t
IOWorker::GetWorkerId(void)
{
    return id;
}

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

int
IOWorker::GetQueueSize(void)
{
    return ioQueue->GetQueueSize();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Adds a UBlockDevice to be controlled by this IOWorker
 * @Param       device: a device to add
 * @return      device count currently added for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::AddDevice(UblockSharedPtr device)
{
    if (nullptr != device)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::IOWORKER_DEVICE_ADDED;
        POS_TRACE_INFO(eventId, "{} has been added to IOWorker{}",
            device->GetName(), id);
        operationQueue.SubmitAndWait(INSERT, device);
    }
    else
    {
        POS_TRACE_WARN(5201, "Try to add null device to user IO worker");
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
IOWorker::AddDevices(std::vector<UblockSharedPtr>* inputList)
{
    uint32_t size = 0;
    for (auto& device : *inputList)
    {
        size = AddDevice(device);
    }

    return size;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis    Removes the given UBlockDevice from the list for this IOWorker
 * @Param       device: a UBlockDevice to remove from the list for this IOWorker
 * @return      device count currently left for this IOWorker.
 */
/* --------------------------------------------------------------------------*/
uint32_t
IOWorker::RemoveDevice(UblockSharedPtr device)
{
    operationQueue.SubmitAndWait(REMOVE, device);

    return deviceList.size();
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

    UbioSmartPtr ubio = nullptr;
    while (false == exit)
    {
        ubio = ioQueue->DequeueUbio();
        while (nullptr != ubio)
        {
            _SubmitAsyncIO(ubio);
            _DoPeriodicJob();
            ubio = ioQueue->DequeueUbio();
        }
        _SubmitPendingIO();
        _DoPeriodicJob();
        usleep(1);
    }
}

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
    UBlockDeviceSubmissionAdapter ublockDeviceSubmission;
    IOWorkerSubmissionNotifier ioWorkerSubmissionNotifier(this);
    qosManager->HandleEventUbioSubmission(&ublockDeviceSubmission,
        &ioWorkerSubmissionNotifier, id, ubio);
}

void
IOWorker::_SubmitPendingIO(void)
{
    UBlockDeviceSubmissionAdapter ublockDeviceSubmission;
    currentOutstandingIOCount -=
        qosManager->IOWorkerPoller(id, &ublockDeviceSubmission);
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
                POS_TRACE_ERROR((uint32_t)POS_EVENT_ID::IOWORKER_UNDERFLOW_HAPPENED,
                    "Command completed more than submitted: current outstanding: {}, completion count: {}",
                    currentOutstandingIOCount, eventCount);
                currentOutstandingIOCount = 0;
            }
        }
    }
}

void
IOWorker::_DoPeriodicJob(void)
{
    _CompleteCommand();
    _HandleDeviceOperation();
}

void
IOWorker::_HandleDeviceOperation(void)
{
    IoWorkerDeviceOperation* operation = operationQueue.Pop();
    if (nullptr == operation)
    {
        return;
    }

    UblockSharedPtr device = operation->GetDevice();
    switch (operation->GetCommand())
    {
        case INSERT:
        {
            /*
                When opening the uram in IOWorker, Uram will issue recovery IO.
                Completion check for this io,
                device must be added to list before device open.
            */
            deviceList.insert(device);
            bool isOpened = device->Open();
            if (false == isOpened)
            {
                deviceList.erase(device);
                DeviceType type = device->GetType();
                if (DeviceType::SSD == type)
                {
                    detachTrigger->Run(device);
                }
            }

            break;
        }
        case REMOVE:
        {
            deviceList.erase(device);
            currentOutstandingIOCount -= device->Close();
            break;
        }
    }
    operation->SetDone();
}

} // namespace pos
