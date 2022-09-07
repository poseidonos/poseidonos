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

#include "src/io/general_io/io_recovery_event.h"

#include "src/array/service/array_service_layer.h"
#include "src/array/service/io_device_checker/i_io_device_checker.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"

namespace pos
{
IoRecoveryEvent::IoRecoveryEvent(UbioSmartPtr ubio, IoCompleter* ioCompleter_)
: ubio(ubio),
  ioCompleter(ioCompleter_)
{
    if (nullptr == ioCompleter)
    {
        ioCompleter = new IoCompleter(ubio);
        ownIoCompleter = true;
    }
    SetEventType(BackendEvent_FrontendIO);
}

IoRecoveryEvent::~IoRecoveryEvent(void)
{
    if (ownIoCompleter && nullptr != ioCompleter)
    {
        delete ioCompleter;
        ioCompleter = nullptr;
        ownIoCompleter = false;
    }
}

bool
IoRecoveryEvent::Execute(void)
{
    IOErrorType errorType = ubio->GetError();

    switch (ubio->dir)
    {
        case UbioDir::Read:
        {
            ubio->ResetError();
            RebuildRead rebuildRead;
            int ret = rebuildRead.Recover(ubio);
            if (unlikely(0 != ret))
            {
                ioCompleter->CompleteUbioWithoutRecovery(errorType, true);
            }
            break;
        }
        case UbioDir::Write:
        {
            int isRecoverable = static_cast<int>(IoRecoveryRetType::SUCCESS);
            IIODeviceChecker* checker = ArrayService::Instance()->Getter()->GetDeviceChecker();
            isRecoverable = checker->IsRecoverable(ubio->GetArrayId(),
                ubio->GetArrayDev(), ubio->GetUBlock());
            if (isRecoverable == static_cast<int>(IoRecoveryRetType::SUCCESS))
            {
                ubio->ResetError();
                ioCompleter->CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);
            }
            else if (isRecoverable == static_cast<int>(IoRecoveryRetType::FAIL))
            {
                ioCompleter->CompleteUbioWithoutRecovery(errorType, true);
            }
            else
            {
                return false;
            }

            break;
        }
        default:
        {
            ioCompleter->CompleteUbioWithoutRecovery(errorType, true);
            break;
        }
    }

    ubio = nullptr;

    return true;
}
} // namespace pos
