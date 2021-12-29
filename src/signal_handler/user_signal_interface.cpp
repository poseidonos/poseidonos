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

#include "src/signal_handler/user_signal_interface.h"

#include <signal.h>

#include "src/signal_handler/signal_handler.h"

namespace pos
{
volatile bool UserSignalInterface::signalOnce = false;
bool UserSignalInterface::enabled = false;
uint64_t UserSignalInterface::ignTimeoutNs = UserSignalInterface::DEFAULT_TIMEOUT;
SystemTimeoutChecker UserSignalInterface::systemTimeoutChecker;

void
UserSignalInterface::Enable(bool inputEnabled)
{
    enabled = inputEnabled;
}

void
UserSignalInterface::SetTimeout(uint64_t inputIgnTimeoutSec)
{
    if (inputIgnTimeoutSec > MAX_TIMEOUT_SEC)
    {
        inputIgnTimeoutSec = MAX_TIMEOUT_SEC;
    }
    if (inputIgnTimeoutSec != 0)
    {
        ignTimeoutNs = inputIgnTimeoutSec * 1000ULL * 1000ULL * 1000ULL;
    }
}

void
UserSignalInterface::TriggerBacktrace(void)
{
    if (enabled)
    {
        if (signalOnce == false)
        {
            signalOnce = true;
            systemTimeoutChecker.SetTimeout(ignTimeoutNs);
            SignalHandlerSingleton::Instance()->ExceptionHandler(SIGUSR1);
        }
        else
        {
            if (systemTimeoutChecker.CheckTimeout() == true)
            {
                systemTimeoutChecker.SetTimeout(ignTimeoutNs);
                SignalHandlerSingleton::Instance()->ExceptionHandler(SIGUSR1);
            }
        }
    }
}

} // namespace pos
