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

#ifndef _INCLUDE_DEVICE_DRIVER_H_
#define _INCLUDE_DEVICE_DRIVER_H_

#include <queue>

#include "mss_ramdisk.h"
#include "mss_ret_code.h"
#include "os_header.h"
#include "src/io/general_io/ubio.h"

#define NVRAM_CAPACITY 2UL * 1024 * 1024 * 1024
#define SSD_CAPACITY 2UL * 1024 * 1024 * 1024

/**
 * Provides fake driver implementaion of device driver.
 */
class DeviceDriver
{
public:
    static DeviceDriver* Instance(MetaStorageType mediaType);
    MssReturnCode SyncIO(ibofos::Ubio* ubio);
    MssReturnCode AsyncIO(ibofos::Ubio* ubio);
    void Flush(void);

    explicit DeviceDriver(MetaStorageType mediaType);
    ~DeviceDriver(void);

private:
    MssRamdisk* mssRamdisk;
    // needed when doing read write
    MetaStorageType mediaType_;
    std::mutex ramLock;
    std::mutex queueLock;
    std::queue<ibofos::Ubio*> requestQueue;
    std::thread asyncHandlerThread;
    bool quitThread;
    int driverState;
    void _AsyncHandlerRead(ibofos::Ubio* ubio);
    void _AsyncHandlerWrite(ibofos::Ubio* ubio);
    void _AsyncRequestHandler(void);
};

#endif // _INCLUDE_DEVICE_DRIVER_H_
