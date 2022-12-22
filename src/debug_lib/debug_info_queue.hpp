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

#ifndef DUMP_MODULE_HPP_
#define DUMP_MODULE_HPP_

#include <stdlib.h>
#include <string.h>

#include <string>

#include "src/debug_lib/dump_manager.h"
#include "src/debug_lib/debug_info_queue.h"

template<typename T>
class DumpSharedPtr;

namespace pos
{

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
template<typename T>
DumpObject<T>::DumpObject(void)
: buffer{},
  userSpecificData(0)
{
    memset(&date, 0x00, sizeof(date));
}

template<typename T>
DumpObject<T>::DumpObject(T& t, uint64_t userSpecific)
: buffer(t),
  userSpecificData(userSpecific)
{
    gettimeofday(&date, NULL);
}

template<typename T>
DumpObject<T>::~DumpObject()
{
}

template<typename T>
DumpObjectPtr<T>::DumpObjectPtr(void)
: buffer{},
  userSpecificData(0)
{
    memset(&date, 0x00, sizeof(date));
}

template<typename T>
DumpObjectPtr<T>::DumpObjectPtr(T& t, uint64_t userSpecific)
: buffer(t),
  userSpecificData(userSpecific)
{
    gettimeofday(&date, NULL);
}

template<typename T>
DumpObjectPtr<T>::~DumpObjectPtr()
{
}

template<typename T>
DebugInfoQueue<T>::DebugInfoQueue()
{
    isEnabled = false;
    entryBufSize = 0;
    entryMaxNum = 0;
}

template<typename T>
DebugInfoQueue<T>::DebugInfoQueue(std::string moduleName, uint32_t num, bool defaultEnable)
{
    entryMaxNum = num;
    entryBufSize = sizeof(T);
    isEnabled = defaultEnable;
    DumpManagerSingleton::Instance()->RegisterDump(moduleName, this);
}

template<typename T>
void
DebugInfoQueue<T>::RegisterDebugInfoQueue(std::string moduleName, uint32_t num, bool defaultEnable)
{
    entryMaxNum = num;
    entryBufSize = sizeof(T);
    isEnabled = defaultEnable;
    DumpManagerSingleton::Instance()->RegisterDump(moduleName, this);
}

template<typename T>
DebugInfoQueue<T>::~DebugInfoQueue()
{
}

template<typename T>
uint64_t
DebugInfoQueue<T>::GetPoolSize()
{
    return entryMaxNum * (entryBufSize + sizeof(DumpObject<T>));
}

template<typename T>
int
DebugInfoQueue<T>::AddDebugInfo(T& t, uint64_t userSpecific, bool lock_enable)
{
    // We assume that there is no simulataneous access to isEnabled.
    if (isEnabled)
    {
        if (lock_enable)
        {
            dumpQueueLock.lock();
        }
        if (dumpQueue.size() == entryMaxNum)
        {
            // Delete Last Entry.
            dumpQueue.pop();
        }
        DumpObject<T> dumpObj(t, userSpecific);
        dumpQueue.push(dumpObj);

        if (lock_enable)
        {
            dumpQueueLock.unlock();
        }

        return 0;
    }
    return -1;
}

template<typename T>
void
DebugInfoQueue<T>::SetEnable(bool enable)
{
    isEnabled = enable;
}

template<typename T>
bool
DebugInfoQueue<T>::IsEnable()
{
    return isEnabled;
}
// LCOV_EXCL_STOP
} // namespace pos
#endif // DUMP_MODULE_HPP_
