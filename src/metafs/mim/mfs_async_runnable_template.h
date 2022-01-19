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
#include "src/metafs/mim/enum_iterator.h"

namespace pos
{
// MetaFS AIO functionality: client <-> metafs <-> mss.pstore
// client <-> metafs: aioable mio
// metafs <-> mss.pstore: aioable mpio
template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
class MetaAsyncRunnable
{
public:
    MetaAsyncRunnable(void);
    virtual ~MetaAsyncRunnable(void);

    virtual void Init(void);
    virtual void RegisterStateHandler(AsyncStateT state, AsyncStateExecutionEntry* entry);
    virtual void ExecuteAsyncState(void* cxt = nullptr);
    virtual void InvokeClientCallback(void);
    virtual void SetAsyncCbCxt(CallbackCxtT* cxt, bool deleteRequired = true);
    CallbackCxtT* GetAsycCbCxt(void);
    AsyncStateT GetPrevState(void);
    AsyncStateT GetCurrState(void);
    AsyncStateT GetStateInExecution(void);
    void SetCurrState(AsyncStateT state);
    AsyncStateT GetNextState(void);
    void SetNextState(AsyncStateT state);
    bool IsCompleted(void);

private:
    virtual void _InitStateHandler(void) = 0;

    AsyncStateExecutionEntry* stateHandler[(int)AsyncStateT::Max];
    CallbackCxtT* cxt;
    AsyncStateT prevState;
    AsyncStateT currState;
    AsyncStateT nextState;
    AsyncStateT stateInExec;
    volatile bool completed;
    bool deleteRequired;
};

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::MetaAsyncRunnable(void)
{
    Init();
    memset(stateHandler, 0, sizeof(AsyncStateExecutionEntry*) * ((int)AsyncStateT::Max));
}

// LCOV_EXCL_START
template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::~MetaAsyncRunnable(void)
{
    for (auto state : Enum<AsyncStateT>())
    {
        if (stateHandler[(int)state])
        {
            delete stateHandler[(int)state];
        }
    }
    if (cxt && deleteRequired)
    {
        delete cxt;
    }
}
// LCOV_EXCL_STOP

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::Init(void)
{
    cxt = nullptr;
    prevState = AsyncStateT::Init;
    currState = AsyncStateT::Init;
    nextState = AsyncStateT::Init;
    stateInExec = AsyncStateT::Init;
    completed = false;
    deleteRequired = true;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::RegisterStateHandler(AsyncStateT state, AsyncStateExecutionEntry* entry)
{
    assert(state < AsyncStateT::Max);
    this->stateHandler[(int)state] = entry;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
CallbackCxtT*
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::GetAsycCbCxt(void)
{
    assert(this->cxt != nullptr);
    return this->cxt;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::SetAsyncCbCxt(CallbackCxtT* cxt, bool deleteRequired)
{
    assert(this->cxt == nullptr); // check whether user sets duplicate aiocb due to incorrect programming
    this->cxt = cxt;
    this->deleteRequired = deleteRequired;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::ExecuteAsyncState(void* _not_used)
{
    bool continueToRun;

    do
    {
        {
            stateInExec = nextState;

            assert(stateHandler[(int)stateInExec] != nullptr);
            AsyncStateExecutionEntry* entry = stateHandler[(int)stateInExec];
            continueToRun = entry->DispatchHandler(entry->GetExpNextState());

            prevState = currState;
            currState = stateInExec;

            if (currState == AsyncStateT::Complete)
            {
                InvokeClientCallback();
                completed = true;
                break;
            }
        }

        if (!continueToRun)
        {
            break;
        }
    } while (true);
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::InvokeClientCallback(void)
{
    if (nullptr != cxt)
    {
        cxt->InvokeCallback();
    }
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
AsyncStateT
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::GetPrevState(void)
{
    return prevState;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
AsyncStateT
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::GetCurrState(void)
{
    return currState;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
AsyncStateT
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::GetStateInExecution(void)
{
    return stateInExec;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::SetCurrState(AsyncStateT state)
{
    currState = state;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
AsyncStateT
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::GetNextState(void)
{
    return nextState;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
void
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::SetNextState(AsyncStateT state)
{
    nextState = state;
}

template<class CallbackCxtT, class AsyncStateT, class AsyncStateExecutionEntry>
bool
MetaAsyncRunnable<CallbackCxtT, AsyncStateT, AsyncStateExecutionEntry>::IsCompleted(void)
{
    return completed;
}
} // namespace pos
