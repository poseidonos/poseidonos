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
#include <cassert>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <queue>

#include "debug_info_maker.h"
#include "debug_info_queue.h"
namespace pos
{

template<typename T>
DebugInfoMaker<T>::DebugInfoMaker(void)
{
    run = false;
    timerUsec = DEFAULT_TIMER_VALUE;
    registered = false;
    infoName = "";
    debugInfoThread = new std::thread(&DebugInfoMaker<T>::_DebugInfoThread, this);
    debugInfoObject = new T;
    isOwner = true;
}

template<typename T>
DebugInfoMaker<T>::DebugInfoMaker(T* t, std::string name, uint32_t entryCount, bool asyncLogging, uint64_t inputTimerUsec, bool enabled)
{
    run = false;
    timerUsec = DEFAULT_TIMER_VALUE;
    registered = false;
    infoName = "";
    debugInfoThread = new std::thread(&DebugInfoMaker<T>::_DebugInfoThread, this);
    debugInfoObject = t;
    isOwner = false;
    RegisterDebugInfo(name, entryCount, asyncLogging, inputTimerUsec, enabled);
}

template<typename T>
DebugInfoMaker<T>::~DebugInfoMaker(void)
{
    run = false;
    if (nullptr != debugInfoThread)
    {
        debugInfoThread->join();
    }
    DeRegisterDebugInfo(infoName);
    if (isOwner == true)
    {
        delete debugInfoObject;
    }
    delete debugInfoThread;
}

template<typename T>
void
DebugInfoMaker<T>::RegisterDebugInfo(std::string name, uint32_t entryCount, bool asyncLogging, uint64_t inputTimerUsec, bool enabled)
{
    if (registered == false)
    {
        if (inputTimerUsec != 0)
        {
            timerUsec = inputTimerUsec;
        }
        debugInfoQueue.RegisterDebugInfoQueue("History_" + name, entryCount, enabled);
        debugInfoQueueForError.RegisterDebugInfoQueue("History_" + name + "_Error", entryCount, enabled);
        debugInfoConcurrentQueue.RegisterDebugInfoQueue("History_" + name + "_Error", entryCount, enabled);
        debugInfoObject->RegisterDebugInfoInstance(name);
        if (asyncLogging)
        {
            run = true;
        }
        registered = true;
        infoName = name;
    }
    else
    {
        assert(0);
    }
}

template<typename T>
void
DebugInfoMaker<T>::DeRegisterDebugInfo(std::string name)
{
    if (registered == true)
    {
        debugInfoQueue.DeRegisterDebugInfoQueue("History_" + name);
        debugInfoQueueForError.DeRegisterDebugInfoQueue("History_" + name + "_Error");
        debugInfoObject->DeRegisterDebugInfoInstance(name);
        registered = false;
    }
}

template<typename T>
void
DebugInfoMaker<T>::AddDebugInfo(uint64_t userSpecific)
{
    assert(registered == true);
    // Source make debug info
    MakeDebugInfo(*debugInfoObject);
    debugInfoObject->instanceOkay = debugInfoObject->IsOkay();
    if ((int)(debugInfoObject->summaryOkay) < (int)(debugInfoObject->instanceOkay))
    {
        debugInfoObject->summaryOkay = debugInfoObject->instanceOkay;
    }
    if (debugInfoObject->instanceOkay != DebugInfoOkay::PASS)
    {
        debugInfoQueueForError.AddDebugInfo(*debugInfoObject, userSpecific);    
    }
    debugInfoQueue.AddDebugInfo(*debugInfoObject, userSpecific);
    bool flushNeeded = false;
    debugInfoConcurrentQueue.AddDebugInfo(*debugInfoObject, userSpecific, flushNeeded);
    if (flushNeeded == true)
    {
        // We will add next revision
        // _Flush();
    }
}

template<typename T>
void
DebugInfoMaker<T>::SetTimer(uint64_t inputTimerUsec)
{
    timerUsec = inputTimerUsec;
}

template<typename T>
void
DebugInfoMaker<T>::_DebugInfoThread(void)
{
    cpu_set_t cpuSet = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    while(run)
    {
        AddDebugInfo(TIMER_TRIGGERED);
        usleep(timerUsec);
    }
}

template<typename T>
bool 
DebugInfoMaker<T>::_AppendToFile(const std::string& filename, const rapidjson::Document& document)
{
    FILE* fp = nullptr;
    // create file if it doesn't exist
    fp = fopen(filename.c_str(), "r"); 
    if (fp == nullptr)
    {
        fp = fopen(filename.c_str(), "w");
        if (fp == nullptr)
        {
            return false;
        }
        fputs("[]", fp);
        fclose(fp);
    }

    // add the document to the file
    fp = fopen(filename.c_str(), "rb+"); 
    if (fp != NULL)
    {
        // check if first is [
        std::fseek(fp, 0, SEEK_SET);
        if (getc(fp) != '[')
        {
            std::fclose(fp);
            return false;
        }

        // is array empty?
        bool isEmpty = false;
        if (getc(fp) == ']')
            isEmpty = true;

        // check if last is ]
        std::fseek(fp, -1, SEEK_END);
        if (getc(fp) != ']')
        {
            std::fclose(fp);
            return false;
        }

        // replace ] by ,
        fseek(fp, -1, SEEK_END);
        if (!isEmpty)
            fputc(',', fp);

        // append the document
        const uint64_t MAX_BUFFER_SIZE_BYTES = 65536;
        char writeBuffer[MAX_BUFFER_SIZE_BYTES];

        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        document.Accept(writer);

        // close the array
        std::fputc(']', fp);
        fclose(fp);
        return true;
    }
    return false;
}

template<typename T>
void
DebugInfoMaker<T>::_Flush(void)
{
    // This phase should be serialized
    // DumpObject contains timestamp and userspecific data.
    DumpObject<T> dumpObject;
    while(debugInfoConcurrentQueue.PopDebugInfo(&dumpObject) == true)
    {
        Document debugInfoDoc(kObjectType);
        Document::AllocatorType& debugInfoAllocator = debugInfoDoc.GetAllocator();
        if (dumpObject.buffer.Serialize(debugInfoDoc) == false)
        {
            continue;
        }
        else
        {
            struct timeval& tv = dumpObject.date;
            time_t timer;
            struct tm *nowtm;
            char tmbuf[64], buf[64];

            timer = tv.tv_sec;
            nowtm = localtime(&timer);
            strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
            snprintf(buf, sizeof(buf), "%s.%06ld", tmbuf, tv.tv_usec);
            debugInfoDoc.AddMember("timestamp", rapidjson::StringRef(buf), debugInfoAllocator);
        }
        _AppendToFile("why.json", debugInfoDoc);
    }
}

} // namespace pos
