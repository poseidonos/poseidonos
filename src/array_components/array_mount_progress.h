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

#include <cstdint>
#include <math.h>

#include "src/logger/logger.h"

namespace pos
{
enum class MountProgressType
{
    MOUNT,
    UNMOUNT,
};

class ArrayMountProgress
{
public:
    void Init(MountProgressType type, int progTotal)
    { 
        progressType = type;
        progressTotal = progTotal; 
    };
    void Update(int prog) { _UpdateProgress(progress + prog); };
    int Get(void)
    {
        if (progress < 0 || progressTotal == 0)
        {
            return progress;
        }
        return (ceil(progress * 100 / progressTotal)); 
    }
    void Set(void) { _UpdateProgress(0); }
    void Reset(void) { progress = NO_ACTIVE_PROGRESS; }

private:
    void _UpdateProgress(int prog)
    {
        progress = prog;
        int eid = EID(ARRAY_MOUNT_PROGRESS);
        string msg = "ARRAY_MOUNT_PROGRESS";
        if (progressType == MountProgressType::UNMOUNT)
        {
            eid = EID(ARRAY_UNMOUNT_PROGRESS);
            msg = "ARRAY_UNMOUNT_PROGRESS";
        }
        POS_REPORT_TRACE(eid, "{}: [{}]", msg, Get());
    }
    const static int NO_ACTIVE_PROGRESS = -1;
    int progress = NO_ACTIVE_PROGRESS;
    MountProgressType progressType = MountProgressType::MOUNT;
    int progressTotal = 0;
};
} // namespace pos
