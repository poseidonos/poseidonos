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

#ifndef VOLUME_LIST_H_
#define VOLUME_LIST_H_

#include <memory>
#include <mutex>
#include <string>

#include "src/volume/volume_base.h"

namespace pos
{
class VolumeList
{
public:
    VolumeList();
    ~VolumeList(void);
    void Clear();
    int Add(VolumeBase* volume);
    int Add(VolumeBase* volume, int id);
    void Remove(int volId);
    int GetID(std::string volName);
    VolumeBase* GetVolume(int volId);
    VolumeBase* GetVolume(std::string volName);
    VolumeBase* Next(int& index);
    int
    Count()
    {
        return volCnt;
    }

    bool IncreasePendingIOCountIfNotZero(int volId, VolumeStatus volumeStatus = VolumeStatus::Mounted, uint32_t ioSubmissionCount = 1);
    void DecreasePendingIOCount(int volId, VolumeStatus volumeStatus = VolumeStatus::Mounted, uint32_t ioCompletionCount = 1);
    void WaitUntilIdle(int volId, VolumeStatus volumeStatus = VolumeStatus::Mounted);
    bool CheckIdleAndSetZero(int volId, VolumeStatus volumeStatus = VolumeStatus::Mounted);
    void InitializePendingIOCount(int volId, VolumeStatus volumeStatus);

private:
    int _NewID();
    int volCnt;
    VolumeBase* items[MAX_VOLUME_COUNT];
    std::mutex listMutex;

    std::atomic<bool> possibleIncreaseIOCount[MAX_VOLUME_COUNT][static_cast<uint32_t>(VolumeStatus::MaxVolumeStatus)];
    std::atomic<uint32_t> pendingIOCount[MAX_VOLUME_COUNT][static_cast<uint32_t>(VolumeStatus::MaxVolumeStatus)];
};

} // namespace pos

#endif // VOLUME_LIST_H_
