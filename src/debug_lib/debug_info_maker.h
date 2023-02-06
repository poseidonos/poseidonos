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
#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <thread>
#include "debug_info_queue.h"
#include "src/cpu_affinity/affinity_manager.h"

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
namespace pos
{

class DebugInfoQueueInstance;
using namespace rapidjson;

enum class DebugInfoOkay
{
    PASS,
    WARNING,
    FAIL,
    MAX_STATUS
};

class DebugInfoInstance
{
public:
    DebugInfoInstance(void);
    DebugInfoInstance(const DebugInfoInstance& debugInfoInstance);
    virtual ~DebugInfoInstance(void);
    virtual void RegisterDebugInfoInstance(std::string str);
    virtual void DeRegisterDebugInfoInstance(std::string str);
    DebugInfoOkay instanceOkay;
    DebugInfoOkay summaryOkay;
    std::string name;
    virtual bool Serialize(Document& doc)
    {
        return false;
    }
    virtual DebugInfoOkay IsOkay(void);

private:
    static std::mutex registeringMutex;
};

extern Document debugInfoDoc;
extern Document::AllocatorType& debugInfoAllocator;

// This define should be used in DebugInfoInstance or child of DebugInfoInstance
#define SERIALIZE(VAR) doc.AddMember(#VAR, VAR, debugInfoAllocator)
template<typename T>
class DebugInfoMaker
{
public:
    DebugInfoMaker(void);
    DebugInfoMaker(T* t, std::string name, uint32_t entryCount, bool asyncLogging = false, uint64_t inputTimerUsec = 0, bool enabled = true);
    virtual ~DebugInfoMaker(void);
    void SetTimer(uint64_t inputTimerUsec);
    virtual void AddDebugInfo(uint64_t userSpecific = 0);
    virtual void RegisterDebugInfo(std::string name, uint32_t entryCount, bool asyncLogging = false, uint64_t inputTimerUsec = 0, bool enabled = true);
    virtual void DeRegisterDebugInfo(std::string name);
protected:
    virtual void MakeDebugInfo(T& obj)
    {
    }
private:
    static const uint64_t DEFAULT_TIMER_VALUE = 2 * 1000ULL * 1000ULL; // 2 sec
    std::atomic<uint64_t> timerUsec;
    void _DebugInfoThread(void);
    void _Flush(void);
    bool _AppendToFile(const std::string& filename, const rapidjson::Document& document);
    T* debugInfoObject;
    std::atomic<bool> registered;
    std::atomic<bool> run;
    DebugInfoQueue<T> debugInfoQueue;
    DebugInfoQueue<T> debugInfoQueueForError;
    DebugInfoConcurrentQueue<T> debugInfoConcurrentQueue;
    std::atomic<bool> isOwner;
    std::thread* debugInfoThread;
    std::string infoName;
    static const uint64_t TIMER_TRIGGERED = 0xFFFFCCCC;
};

extern std::map<std::string, DebugInfoInstance*> debugInfo;

} // namespace pos
