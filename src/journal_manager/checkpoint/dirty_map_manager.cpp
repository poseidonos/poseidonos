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

#include "dirty_map_manager.h"

#include "assert.h"
#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
DirtyMapManager::~DirtyMapManager(void)
{
    for (auto it : pendingDirtyMaps)
    {
        delete it;
    }
    pendingDirtyMaps.clear();
}

void
DirtyMapManager::Init(JournalConfiguration* journalConfiguration)
{
    int numLogGroups = journalConfiguration->GetNumLogGroups();
    for (int id = 0; id < numLogGroups; id++)
    {
        pendingDirtyMaps.push_back(new DirtyMapList());
    }
}

// Init function for unit test
// TODO (huijeong.kim) move initialization to the construcor
void
DirtyMapManager::Init(std::vector<DirtyMapList*> dirtyPages)
{
    pendingDirtyMaps = dirtyPages;
}

void
DirtyMapManager::Dispose(void)
{
    for (auto it : pendingDirtyMaps)
    {
        delete it;
    }
    pendingDirtyMaps.clear();
}

MapList
DirtyMapManager::GetDirtyList(int logGroupId)
{
    assert(logGroupId < static_cast<int>(pendingDirtyMaps.size()));
    return pendingDirtyMaps[logGroupId]->GetList();
}

MapList
DirtyMapManager::GetTotalDirtyList(void)
{
    MapList dirtyMaps;
    for (auto it : pendingDirtyMaps)
    {
        MapList dirtyMap = it->PopDirtyList();
        dirtyMaps.insert(dirtyMap.begin(), dirtyMap.end());
    }
    return dirtyMaps;
}

void
DirtyMapManager::DeleteDirtyList(int volumeId)
{
    for (auto it = pendingDirtyMaps.begin(); it != pendingDirtyMaps.end(); ++it)
    {
        (*it)->Delete(volumeId);
    }
}

void
DirtyMapManager::LogFilled(int logGroupId, MapList& dirty)
{
    pendingDirtyMaps[logGroupId]->Add(dirty);
}

void
DirtyMapManager::LogBufferReseted(int logGroupId)
{
    assert(logGroupId < static_cast<int>(pendingDirtyMaps.size()));
    pendingDirtyMaps[logGroupId]->Reset();
}

} // namespace pos
