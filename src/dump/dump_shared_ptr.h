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

#ifndef DUMP_SHARED_PTR_H_
#define DUMP_SHARED_PTR_H_

#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/lib/singleton.h"

enum class DumpSharedPtrType
{
    UBIO,
    IO_CONTEXT,
    CALLBACK,
    JOURNAL_IO_CONTEXT,
    MAX_DUMP_PTR
};

namespace pos
{
template<typename T, int>
class DumpSharedPtr
{
public:
    static void* operator new(std::size_t size);
    static void operator delete(void* ptr);
    static void operator delete(void* ptr, std::size_t size);
    static void* operator new[](std::size_t size);
    static void operator delete[](void* ptr);
    static void operator delete[](void* ptr, std::size_t size);

private:
    static void* _New(std::size_t size);
    static void _Delete(void* ptr);
};

class T;

template<typename T, int>
class DumpSharedModule : public DumpModule<T>
{
public:
    DumpSharedModule(std::string moduleName, bool defaultEnable);
    ~DumpSharedModule(void) override;
    std::unordered_map<uint64_t, DumpObject<T>> dumpMap;
    int Add(T t, bool lock_enable = true);
    int Delete(T t, bool lock_enable = true);
};

class DumpSharedModuleInstanceEnable
{
public:
    static bool debugLevelEnable;
};

template<typename T, int moduleNumber>
class DumpSharedModuleInstance
{
public:
    DumpSharedModuleInstance(void);
    ~DumpSharedModuleInstance(void);
    DumpSharedModule<T, moduleNumber>* DumpInstance(void);

private:
    DumpSharedModule<T, moduleNumber>* dumpSharedInstance;
};

template<typename T, int moduleNumber>
using DumpSharedModuleInstanceSingleton =
    Singleton<DumpSharedModuleInstance<T, moduleNumber>>;
extern void* gDumpSharedModulePtr[static_cast<int>(DumpSharedPtrType::MAX_DUMP_PTR)];

} // namespace pos

#endif // DUMP_SHARED_PTR_H_
