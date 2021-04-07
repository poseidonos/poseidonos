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

#pragma once

#include <mutex>
#include <vector>

#include "../log_write/buffer_write_done_notifier.h"
#include "src/mapper/mpage_info.h"

namespace ibofos
{
class DirtyPageList
{
public:
    DirtyPageList(void);

    void Add(MapPageList& dirty);
    MapPageList GetList(void);
    void Reset(void);
    void Delete(int volumeId);

private:
    std::mutex dirtyListLock;
    MapPageList dirtyPages;
};

class DirtyMapManager : public LogBufferWriteDoneEvent
{
public:
    virtual ~DirtyMapManager(void);

    void Init(int numLogGroups);

    MapPageList GetDirtyList(int logGroupId);
    void DeleteDirtyList(int volumeId);

    virtual void LogFilled(int logGroupId, MapPageList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

private:
    void _Reset(int logGroupId);

    std::vector<DirtyPageList*> pendingDirtyPages;
};

} // namespace ibofos
