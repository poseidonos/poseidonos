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

#include "array.h"

namespace pos
{
Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, IStateControl* iState)
: devMgr_(new ArrayDeviceManager(nullptr, name))
{
}

int
Array::Load()
{
    return 0;
}

Array::~Array(void)
{
    delete devMgr_;
}

int
Array::Create(DeviceSet<string> nameSet, string dataRaidType)
{
    return 0;
}

int
Array::Init(void)
{
    return 0;
}

void
Array::Dispose(void)
{
}

int
Array::Delete()
{
    return 0;
}

int
Array::AddSpare(string spare)
{
    return 0;
}

int
Array::RemoveSpare(string devName)
{
    return 0;
}

void
UpdateArrayState()
{
}

const PartitionLogicalSize*
Array::GetSizeInfo(PartitionType type)
{
    return nullptr;
}

DeviceSet<string>
Array::GetDevNames()
{
    DeviceSet<string> nameSet;
    return nameSet;
}

int
Array::DetachDevice(UblockSharedPtr uBlock)
{
    return 0;
}

} // namespace pos
