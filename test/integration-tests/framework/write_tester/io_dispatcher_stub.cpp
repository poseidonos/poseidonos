/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "test/integration-tests/framework/write_tester/io_dispatcher_stub.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/io_error_type.h"
#include "src/event_scheduler/io_completer.h"
#include "src/logger/logger.h"

namespace pos
{
IODispatcherStub::IODispatcherStub(void)
{

}

IODispatcherStub::IODispatcherStub(EventScheduler* eventScheduler_)
:  IODispatcher(nullptr, eventScheduler_)
{

}

IODispatcherStub::~IODispatcherStub(void)
{

}

int
IODispatcherStub::Submit(UbioSmartPtr ubio, bool sync, bool ioRecoveryNeeded)
{
    // TODO:
    // IoCompleter ioCompleter {ubio, eventScheduler, nullptr};
    // ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);
    if (nullptr != ubio)
    {
        ubio->Complete(IOErrorType::SUCCESS);
        CallbackSmartPtr callback = ubio->GetCallback();
        callback->Execute();

        POS_TRACE_DEBUG(EID(VOLUMEIO_DEBUG_SUBMIT),
            "callback event type {} successfully executed.", callback->GetEventType());
    }

    // TODO: POS_IO_STATUS_SUCCESS
    return 0;
}
} // namespace pos
