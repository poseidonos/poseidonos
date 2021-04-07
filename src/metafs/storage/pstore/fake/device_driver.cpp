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

#include "device_driver.h"

#include <future>
#include <thread>

#define MOCK_DRIVER 1
#define SLEEP_TIME (0)
// Can change it to META_PAGE_SIZE_IN_BYTES
const unsigned int PAGE_SIZE = ibofos::PageSize;

/**
 * Single global instance of DeviceDriver class
 */

/**
 * Provides fake implementation of sync IO on device.
 * Fault tolerance layer will be used instead of this.
 */
MssReturnCode
DeviceDriver::SyncIO(ibofos::Ubio* ubio)
{
    unsigned char* data;
    uint64_t lba;
#ifdef MOCK_DRIVER
    std::cout << "DD " << ubio->address << " " << ubio->size << "\n";
    std::cout << "#### Mock dev driver IO Done.\n";
    return MssReturnCode::Success;
#endif

    if (driverState == 0)
    {
        std::cout << "unable to init \n";
        return MssReturnCode::Failed;
    }

    if (ubio->dir == ibofos::UbioDir::Write)
    {
        ramLock.lock();
        for (int i = 0; i < ubio->size / PAGE_SIZE; i++)
        {
            if (MssReturnCode::Failed == mssRamdisk->WritePage(mediaType_, ubio->address + i, ubio->GetBuffer2(i), 1))
            {
                ubio->error = 1;
                ramLock.unlock();
                return MssReturnCode::Failed;
            }
        }
        ramLock.unlock();
    }
    else if (ubio->dir == ibofos::UbioDir::Read)
    {
        ramLock.lock();
        for (int i = 0; i < ubio->size / PAGE_SIZE; i++)
        {
            if (MssReturnCode::Failed == mssRamdisk->ReadPage(mediaType_, ubio->address + i, ubio->GetBuffer2(i), 1))
            {
                ubio->error = 1;
                ramLock.unlock();
                return MssReturnCode::Failed;
            }
        }
        ramLock.unlock();
    }
    else
    {
        // Wrong command
        return MssReturnCode::Failed;
    }
    return MssReturnCode::Success;
}

/**
 * private function to handle read requests
 *
 * @ubio request information
 */
void
DeviceDriver::_AsyncHandlerRead(ibofos::Ubio* ubio)
{
    unsigned char* data;

    ramLock.lock();

    // As memory is already preallocated in beginning , so no pointer check
    for (int i = 0; i < ubio->size / ibofos::PageSize; i++)
    {
        if (MssReturnCode::Failed == mssRamdisk->ReadPage(mediaType_, ubio->address + i, ubio->GetBuffer(i), 1))
        {
            ubio->error = 1;
        }
        /*data = dataMap[ubio->address + i];
          memcpy( (char*) ubio->GetBuffer(i), (char*) data , ibofos::PageSize);
          */
    }

    ramLock.unlock();

    // injecting sleep time
    usleep(SLEEP_TIME);
    // status callback
    (*ubio->endio)(ubio);
}

/**
 * private function to handle write requests
 *
 * @ubio request information
 */
void
DeviceDriver::_AsyncHandlerWrite(ibofos::Ubio* ubio)
{
    unsigned char* data;

    ramLock.lock();

    // As memory is already preallocated in beginning , so no pointer check
    for (int i = 0; i < ubio->size / ibofos::PageSize; i++)
    {
        if (MssReturnCode::Failed == mssRamdisk->ReadPage(mediaType_, ubio->address + i, ubio->GetBuffer(i), 1))
        {
            ubio->error = 1;
        }
    }

    ramLock.unlock();

    // injecting sleep time
    usleep(SLEEP_TIME);

    // status callback
    (*ubio->endio)(ubio);
}

void
DeviceDriver::_AsyncRequestHandler(void)
{
    ibofos::Ubio* request;
    do
    {
        request = nullptr;
        queueLock.lock();
        if (!requestQueue.empty())
        {
            request = requestQueue.front();
            requestQueue.pop();
        }
        queueLock.unlock();
        if (request != nullptr)
        {
            if (request->dir == ibofos::UbioDir::Read)
            {
                _AsyncHandlerRead(request);
            }
            else
            {
                _AsyncHandlerWrite(request);
            }
        }
    } while (!quitThread);
}

/** 
 * Provides fake implementation of Async IO on device.
 * Fault tolerance layer will be used instead of this.
 *
 * @ubio page information
 *
 */
MssReturnCode
DeviceDriver::AsyncIO(ibofos::Ubio* ubio)
{
    // Make is look like this thread is submitting request to
    // other thread. So new thread is created on each request.
    // And control will be returned to caller function.
    if (ubio->dir == ibofos::UbioDir::Read || ubio->dir == ibofos::UbioDir::Write)
    {
        queueLock.lock();
        requestQueue.push(ubio);
        queueLock.unlock();
    }
    else
    {
        // Wrong case
        ubio->error = 1;
        return MssReturnCode::Failed;
    }
    // submitted
    return MssReturnCode::Success;
}

DeviceDriver*
DeviceDriver::Instance(MetaStorageType mediaType)
{
    if (MetaStorageType::SSD == mediaType)
    {
        return nullptr;
    }
    else if (MetaStorageType::NVRAM == mediaType)
    {
        return nullptr;
    }
    return nullptr;
}

void
DeviceDriver::Flush(void)
{
    while (!requestQueue.empty())
    {
    }
    return;
}

/**
 * Constructor
 */
DeviceDriver::DeviceDriver(MetaStorageType mediaType)
{
    driverState = 1;
    mediaType_ = mediaType;
    ramLock.unlock();
    queueLock.unlock();
    quitThread = false;
}

/**
 * Destructor
 */
DeviceDriver::~DeviceDriver(void)
{
    // Wait for all request to complete , then close thread
    while (!requestQueue.empty())
    {
    }
    quitThread = true;
}
