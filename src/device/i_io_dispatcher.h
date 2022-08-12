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

#pragma once

#include <memory>
#include "src/include/smart_ptr_type.h"
namespace pos
{

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Decide IOWorker for handling bio
 */
/* --------------------------------------------------------------------------*/
class IIODispatcher
{
public:
    IIODispatcher(void);
    virtual ~IIODispatcher(void);

    virtual void AddIOWorker(cpu_set_t cpuSet) = 0;
    virtual void RemoveIOWorker(cpu_set_t cpuSet) = 0;

    // These functions shall be guarrenteed to run itself at once by caller.
    virtual void AddDeviceForReactor(UblockSharedPtr dev) = 0;
    virtual void RemoveDeviceForReactor(UblockSharedPtr dev) = 0;

    virtual void AddDeviceForIOWorker(UblockSharedPtr dev, cpu_set_t cpuSet) = 0;
    virtual void RemoveDeviceForIOWorker(UblockSharedPtr dev) = 0;

    virtual int Submit(UbioSmartPtr ubio, bool sync = false, bool ioRecoveryNeeded = true) = 0;
    virtual void ProcessQueues(void) = 0;
};

} // namespace pos
