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

#include "array_mount_progress.h"

namespace pos
{
void ArrayMountProgress::Init(string name, MountProgressType type, int progTotal)
{
    arrayName = name;
    progressType = type;
    progressTotal = progTotal; 
}

void ArrayMountProgress::Update(int prog)
{ 
    _UpdateProgress(progress + prog); 
}

int ArrayMountProgress::Get(void)
{
    if (progress < 0 || progressTotal == 0)
    {
        return progress;
    }
    return (ceil(progress * 100 / progressTotal)); 
}

void ArrayMountProgress::Set(void)
{
    _UpdateProgress(0);
}

void ArrayMountProgress::Reset(void)
{
    progress = NO_ACTIVE_PROGRESS;
}

void ArrayMountProgress::_UpdateProgress(int prog)
{
    progress = prog;
    if (progressType == MountProgressType::MOUNT)
    {
        int eid = EID(ARRAY_MOUNT_PROGRESS);
        string msg = "ARRAY_MOUNT_PROGRESS";
        POS_REPORT_TRACE(eid, "{}, [{}], array_name:{}", msg, Get(), arrayName);
    }
    else if (progressType == MountProgressType::UNMOUNT)
    {
        int eid = EID(ARRAY_UNMOUNT_PROGRESS);
        string msg = "ARRAY_UNMOUNT_PROGRESS";
        POS_REPORT_TRACE(eid, "{}, [{}], array_name:{}", msg, Get(), arrayName);
    }
    else
    {
        assert(false);
        POS_TRACE_ERROR(EID(ARRAY_MOUNT_PROGRESS), "unsupported progress type");
    }
}
} // namespace pos
