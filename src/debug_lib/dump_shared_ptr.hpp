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

#ifndef DUMP_SHARED_PTR_HPP_
#define DUMP_SHARED_PTR_HPP_

#include <cstring>
#include <string>

#include "src/debug_lib/dump_manager.h"
#include "src/debug_lib/dump_shared_ptr.h"
#include "src/include/pos_event_id.h"
#include "src/memory_checker/memory_checker.h"

namespace pos
{

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
template<typename T, int moduleNumber>
void*
DumpSharedPtr<T, moduleNumber>::_New(std::size_t size)
{
    T ptr = static_cast<T>(MemoryChecker::New(size));

    DumpSharedModuleInstanceSingleton<T, moduleNumber>::Instance()->DumpInstance()->Add(ptr);

    return ptr;
}

template<typename T, int moduleNumber>
void
DumpSharedPtr<T, moduleNumber>::_Delete(void* ptr)
{
    DumpSharedModuleInstanceSingleton<T, moduleNumber>::Instance()->DumpInstance()->Delete((T)(ptr));

    MemoryChecker::Delete(ptr);
}

template<typename T, int moduleNumber>
void*
DumpSharedPtr<T, moduleNumber>::operator new(std::size_t size)
{
    return _New(size);
}

template<typename T, int moduleNumber>
void
DumpSharedPtr<T, moduleNumber>::operator delete(void* ptr)
{
    _Delete(ptr);
}

template<typename T, int moduleNumber>
void*
DumpSharedPtr<T, moduleNumber>::operator new[](std::size_t size)
{
    return _New(size);
}

template<typename T, int moduleNumber>
void
DumpSharedPtr<T, moduleNumber>::operator delete[](void* ptr)
{
    _Delete(ptr);
}

template<typename T, int moduleNumber>
void
DumpSharedPtr<T, moduleNumber>::operator delete(void* ptr, std::size_t size)
{
    _Delete(ptr);
}

template<typename T, int moduleNumber>
void
DumpSharedPtr<T, moduleNumber>::operator delete[](void* ptr, std::size_t size)
{
    _Delete(ptr);
}

template<typename T, int moduleNumber>
DumpSharedModule<T, moduleNumber>::DumpSharedModule(std::string moduleName, bool defaultEnable)
{
    this->entryBufSize = sizeof(T);
    this->isEnabled = defaultEnable;
    DumpManagerSingleton::Instance()->RegisterDump(moduleName, this);
}

template<typename T, int moduleNumber>
DumpSharedModuleInstance<T, moduleNumber>::DumpSharedModuleInstance(void)
{
    std::string str = "SharedPointer";
    str += to_string(moduleNumber);

    if (moduleNumber < static_cast<int>(DumpSharedPtrType::MAX_DUMP_PTR))
    {
        dumpSharedInstance = new DumpSharedModule<T, moduleNumber>(str, true);
        gDumpSharedModulePtr[moduleNumber] = (void*)dumpSharedInstance;
    }
}

template<typename T, int moduleNumber>
DumpSharedModuleInstance<T, moduleNumber>::~DumpSharedModuleInstance(void)
{
    delete dumpSharedInstance;
}

template<typename T, int moduleNumber>
DumpSharedModule<T, moduleNumber>*
DumpSharedModuleInstance<T, moduleNumber>::DumpInstance(void)
{
    return dumpSharedInstance;
}

template<typename T, int moduleNumber>
DumpSharedModule<T, moduleNumber>::~DumpSharedModule(void)
{
}

template<typename T, int moduleNumber>
int
DumpSharedModule<T, moduleNumber>::Add(T t, bool lock_enable)
{
    // We assume that there is no simulataneous access to isEnabled.

    if (DumpSharedModuleInstanceEnable::debugLevelEnable)
    {
        if (lock_enable)
        {
            this->dumpQueueLock.lock();
        }

        uint64_t key = reinterpret_cast<uint64_t>(t);

        if (dumpMap.find(key) == dumpMap.end())
        {
            DumpObjectPtr<T> dumpObj(t, 0);
            dumpMap[key] = dumpObj;
        }

        if (lock_enable)
        {
            this->dumpQueueLock.unlock();
        }
        return 0;
    }
    return -1;
}

template<typename T, int moduleNumber>
int
DumpSharedModule<T, moduleNumber>::Delete(T t, bool lock_enable)
{
    // We assume that there is no simulataneous access to isEnabled.

    if (DumpSharedModuleInstanceEnable::debugLevelEnable)
    {
        if (lock_enable)
        {
            this->dumpQueueLock.lock();
        }

        uint64_t key = reinterpret_cast<uint64_t>(t);

        if ((dumpMap.find(key) != dumpMap.end()))
        {
            dumpMap.erase(key);
        }

        if (lock_enable)
        {
            this->dumpQueueLock.unlock();
        }

        return 0;
    }
    return -1;
}
// LCOV_EXCL_STOP

} // namespace pos

#endif // DUMP_SHARED_PTR_HPP_
