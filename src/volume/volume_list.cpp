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

#include "src/volume/volume_list.h"

#include <string>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

VolumeList::VolumeList()
{
    volCnt = 0;

    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        items[i] = nullptr;
    }
}

VolumeList::~VolumeList(void)
{
    Clear();
}

void
VolumeList::Clear()
{
    std::unique_lock<std::mutex> lock(listMutex);
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr)
        {
            delete items[i];
            items[i] = nullptr;
        }
    }
    volCnt = 0;
}

int
VolumeList::Add(VolumeBase* volume)
{
    if (volCnt == MAX_VOLUME_COUNT)
    {
        POS_TRACE_WARN(EID(CREATE_VOL_EXCEED_MAX_NUM_OF_VOLS), "curr: {}, max: {}", volCnt, MAX_VOLUME_COUNT);
        return EID(CREATE_VOL_EXCEED_MAX_NUM_OF_VOLS);
    }

    std::unique_lock<std::mutex> lock(listMutex);
    int id = _NewID();
    if (id < 0)
    {
        return EID(VOL_INTERNAL_ID_ALLOC_FAILED);
    }
    volume->ID = id;
    items[id] = volume;
    volCnt++;
    InitializePendingIOCount(id, VolumeStatus::Unmounted);
    POS_TRACE_DEBUG(EID(SUCCESS), "Volume added to the list, VOL_CNT: {}, VOL_ID: {}", volCnt, id);
    return EID(SUCCESS);
}

int
VolumeList::Add(VolumeBase* volume, int id)
{
    std::unique_lock<std::mutex> lock(listMutex);
    if (items[id] == nullptr)
    {
        volume->ID = id;
        items[id] = volume;
        volCnt++;
        InitializePendingIOCount(id, VolumeStatus::Unmounted);
        POS_TRACE_DEBUG(EID(VOL_DEBUG_MSG), "Volume added to the list, VOL_CNT: {}, VOL_ID: {}", volCnt, id);
        return EID(SUCCESS);
    }

    POS_TRACE_ERROR(EID(VOL_INTERNAL_ID_DUPLICATION), "The same ID volume exists");
    return EID(VOL_INTERNAL_ID_DUPLICATION);
}

void
VolumeList::Remove(int volId)
{
    if (volId < 0 || volId >= MAX_VOLUME_COUNT)
    {
        POS_TRACE_ERROR(EID(VOL_INTERNAL_INVALID_ID), "Invalid index error");
        throw EID(VOL_INTERNAL_INVALID_ID);
    }

    std::unique_lock<std::mutex> lock(listMutex);
    VolumeBase* target = items[volId];
    if (target == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "vol_id: {}", volId);
        throw EID(VOL_NOT_FOUND);
    }

    delete target;
    items[volId] = nullptr;
    volCnt--;

    POS_TRACE_INFO(EID(VOL_DEBUG_MSG), "Volume removed from the list VOL_CNT {}", volCnt);
}

int
VolumeList::_NewID()
{
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] == nullptr)
        {
            POS_TRACE_DEBUG(EID(SUCCESS), "Volume New ID: {}", i);
            return i;
        }
    }
    return -1;
}

int
VolumeList::GetID(std::string volName)
{
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr && items[i]->GetName() == volName)
        {
            return i;
        }
    }
    return -1;
}

VolumeBase*
VolumeList::GetVolume(std::string volName)
{
    int volId = GetID(volName);
    if (volId >= 0)
    {
        return items[volId];
    }
    return nullptr;
}

VolumeBase*
VolumeList::GetVolume(int volId)
{
    VolumeBase* volume = nullptr;

    if (likely((0 <= volId) && (volId < MAX_VOLUME_COUNT)))
    {
        volume = items[volId];
    }

    return volume;
}

VolumeBase*
VolumeList::Next(int& index)
{
    for (int i = index + 1; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr)
        {
            index = i;
            return items[i];
        }
    }

    return nullptr;
}

void
VolumeList::InitializePendingIOCount(int volId, VolumeStatus volumeStatus)
{
    uint32_t index = static_cast<uint32_t>(volumeStatus);
    pendingIOCount[volId][index] = 1;
    possibleIncreaseIOCount[volId][index] = true;
}

// This function check possibleIncreaseIOCount before increase pendingIO Count to avoid waiting infinite IO from HOST (or Internal Module)
// Even if possibleIncreaseIOCount is false, we need to check pendingIOCount == 0 or not
// because there is a possibility that calls in sequence of "WaitUntilIdle => IncreasePendingIOCountIfoNozero"

bool
VolumeList::IncreasePendingIOCountIfNotZero(int volId, VolumeStatus volumeStatus, uint32_t ioSubmissionCount)
{
    uint32_t index = static_cast<uint32_t>(volumeStatus);
    if (unlikely (possibleIncreaseIOCount[volId][index] == false))
    {
        return false;
    }
    uint32_t oldPendingIOCount = pendingIOCount[volId][index].load();
    do
    {
        if (unlikely(oldPendingIOCount == 0))
        {
            // already volume base is deleted
            return false;
        }
    } while (!pendingIOCount[volId][index].compare_exchange_weak(oldPendingIOCount, oldPendingIOCount + ioSubmissionCount));

    if (unlikely((UINT32_MAX - oldPendingIOCount) < ioSubmissionCount))
    {
        POS_TRACE_ERROR(EID(VOL_INTERNAL_UNEXPECTED_PENDING_IO_COUNT),
            "PendingIOCount overflow!!: Current PendingIOCount: {}, "
            "Submission Count: {}",
            oldPendingIOCount,
            ioSubmissionCount);
        return false;
    }
    return true;
}

void
VolumeList::DecreasePendingIOCount(int volId, VolumeStatus volumeStatus, uint32_t ioCompletionCount)
{
    uint32_t index = static_cast<uint32_t>(volumeStatus);
    uint32_t oldPendingIOCount = pendingIOCount[volId][index].fetch_sub(ioCompletionCount,
        memory_order_relaxed);
    if (unlikely(oldPendingIOCount < ioCompletionCount))
    {
        POS_TRACE_ERROR(EID(VOL_INTERNAL_UNEXPECTED_PENDING_IO_COUNT),
            "PendingIOCount underflow!!: Current PendingIOCount: {}, "
            "Completion Count: {}",
            oldPendingIOCount,
            ioCompletionCount);
    }
}

bool
VolumeList::CheckIdleAndSetZero(int volId, VolumeStatus volumeStatus)
{
    uint32_t index = static_cast<uint32_t>(volumeStatus);
    uint32_t oldPendingIOCount = pendingIOCount[volId][index].load();
    // If oldPendingIOCount is greater than 0
    // If oldPendingIOCount == 1, decrease and return idle as true.
    do
    {
        if (unlikely(oldPendingIOCount > 1))
        {
            return false;
        }
    } while (!pendingIOCount[volId][index].compare_exchange_weak(oldPendingIOCount, oldPendingIOCount - 1));
    assert(oldPendingIOCount == 1);
    return true;
}

void
VolumeList::WaitUntilIdle(int volId, VolumeStatus volumeStatus)
{
    possibleIncreaseIOCount[volId][volumeStatus] = false;
    while (false == CheckIdleAndSetZero(volId, volumeStatus))
    {
        usleep(1);
    }
}




} // namespace pos
