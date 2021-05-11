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

#include "dirty_page_list.h"

#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
DirtyPageList::DirtyPageList(void)
{
    dirtyPages.clear();
}

void
DirtyPageList::Add(MapPageList& dirty)
{
    std::unique_lock<std::mutex> lock(dirtyListLock);

    for (auto it = dirty.begin(); it != dirty.end(); it++)
    {
        int mapId = it->first;

        auto mapIt = dirtyPages.find(mapId);
        if (mapIt != dirtyPages.end())
        {
            dirtyPages[mapId].insert((it->second.begin()), it->second.end());
        }
        else
        {
            dirtyPages.emplace(mapId, it->second);
        }
    }
}

MapPageList
DirtyPageList::GetList(void)
{
    return dirtyPages;
}

void
DirtyPageList::Reset(void)
{
    dirtyPages.clear();
}

void
DirtyPageList::Delete(int volumeId)
{
    std::unique_lock<std::mutex> lock(dirtyListLock);

    auto mapIt = dirtyPages.find(volumeId);
    if (mapIt != dirtyPages.end())
    {
        (mapIt->second).clear();
        dirtyPages.erase(volumeId);
    }
}

} // namespace pos
