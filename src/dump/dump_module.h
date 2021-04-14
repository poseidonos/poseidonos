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

#ifndef DUMP_MODULE_H_
#define DUMP_MODULE_H_

#include <sys/time.h>
#include <unistd.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "src/dump/dump_buffer.h"

namespace pos
{
template<typename T>
class DumpObject
{
public:
    DumpObject();
    DumpObject(T& t, uint64_t userSpecific);
    ~DumpObject();
    T buffer;
    uint64_t userSpecificData;
    struct timeval date; // 8 byte
};

class AbstractDumpModule
{
public:
    virtual void SetEnable(bool enable) = 0;
    virtual bool IsEnable() = 0;
    virtual uint32_t GetPoolSize() = 0;
};

template<typename T>
class DumpModule : public AbstractDumpModule
{
public:
    DumpModule();

    DumpModule(std::string moduleName,
        uint32_t num, bool enable);
    virtual ~DumpModule();
    std::mutex dumpQueueLock;
    int AddDump(T& t, uint64_t userSpecific, bool lock_enable = true);
    virtual void SetEnable(bool enable);
    virtual bool IsEnable();
    virtual uint32_t GetPoolSize();

    static const int MAX_ENTRIES_FOR_READHANDLER_CONSTRUCTOR_UBIO = 10;
    static const int MAX_ENTRIES_FOR_WRITEHANDLER_CONSTRUCTOR_UBIO = 10;
    static const int MAX_ENTRIES_FOR_AIO_ENDIO_MEMDUMP = 10;
    static const int MAX_ENTRIES_FOR_WRITEHANDLER_CONSTRUCTOR_MEMDUMP = 10;

protected:
    bool isEnabled;
    std::queue<DumpObject<T>> dumpQueue;
    uint32_t entryBufSize;
    uint32_t entryMaxNum;
};

} // namespace pos
#endif // DUMP_MODULE_H_
