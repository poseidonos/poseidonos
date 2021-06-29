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

#include "volume_event_publisher.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <iostream>
#include <utility>

namespace pos
{
VolumeEventPublisher::VolumeEventPublisher()
{
}

VolumeEventPublisher::~VolumeEventPublisher()
{
    subscribers.clear();
}

void
VolumeEventPublisher::RegisterSubscriber(VolumeEvent* subscriber, std::string arrayName)
{
    subscribers.push_back(std::pair<std::string, VolumeEvent*>(arrayName, subscriber));
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "VolumeEvent subscriber {} is registered", subscriber->Tag());
}

void
VolumeEventPublisher::RegisterNvmfTargetSubscriber(VolumeEvent* subscriber, std::string arrayName)
{
    subscribers.insert(subscribers.begin(), std::pair<std::string, VolumeEvent*>(arrayName, subscriber));
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "VolumeEvent subscriber {} is registered", subscriber->Tag());
}

void
VolumeEventPublisher::RemoveSubscriber(VolumeEvent* subscriber, std::string arrayName)
{
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            if (it->second == subscriber)
            {
                POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                    "VolumeEvent subscriber {} is removed", it->second->Tag());
                subscribers.erase(it);
                break;
            }
        }
    }
}

bool
VolumeEventPublisher::NotifyVolumeCreated(std::string volName, int volID,
    uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeCreated, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.rbegin(); it != subscribers.rend(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeCreated to {} : {} {} {} {} {}",
                it->second->Tag(), volName, volID, volSizeByte, maxiops, maxbw);
            bool res = it->second->VolumeCreated(volName, volID, volSizeByte, maxiops, maxbw, arrayName);
            if (res == false)
            {
                ret = false;
            }

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeCreated to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

bool
VolumeEventPublisher::NotifyVolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeUpdated, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeUpdated to {} : {} {} {} {}",
                it->second->Tag(), volName, volID, maxiops, maxbw);
            bool res = it->second->VolumeUpdated(volName, volID, maxiops, maxbw, arrayName);
            if (res == false)
            {
                ret = false;
            }

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeUpdated to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

bool
VolumeEventPublisher::NotifyVolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeDeleted, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeDeleted to {} : {} {} {}",
                it->second->Tag(), volName, volID, volSizeByte);
            bool res = it->second->VolumeDeleted(volName, volID, volSizeByte, arrayName);
            if (res == false)
            {
                ret = false;
            }

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeDeleted to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

bool
VolumeEventPublisher::NotifyVolumeMounted(std::string volName, std::string subnqn,
    int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeMounted, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.rbegin(); it != subscribers.rend(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeMounted to {} : {} {} {} {} {} {}",
                it->second->Tag(), volName, subnqn, volID, volSizeByte, maxiops, maxbw);
            bool res = it->second->VolumeMounted(volName, subnqn, volID, volSizeByte, maxiops, maxbw, arrayName);
            if (res == false)
            {
                ret = false;
            }
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeMounted to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

bool
VolumeEventPublisher::NotifyVolumeUnmounted(std::string volName, int volID, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeUnmounted, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeUnmounted to {} : {} {}",
                it->second->Tag(), volName, volID);
            bool res = it->second->VolumeUnmounted(volName, volID, arrayName);
            if (res == false)
            {
                ret = false;
            }

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeUnmounted to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

bool
VolumeEventPublisher::NotifyVolumeLoaded(std::string name, int id,
    uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeLoaded, # of subscribers: {}", subscribers.size());

    bool ret = true;
    for (auto it = subscribers.rbegin(); it != subscribers.rend(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeLoaded to {} : {} {} {} {} {}",
                it->second->Tag(), name, id, totalSize, maxiops, maxbw);
            bool res = it->second->VolumeLoaded(name, id, totalSize, maxiops, maxbw, arrayName);
            if (res == false)
            {
                ret = false;
            }

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeLoaded to {} done, res: {}",
                it->second->Tag(), res);
        }
    }
    return ret;
}

void
VolumeEventPublisher::NotifyVolumeDetached(vector<int> volList, std::string arrayName)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
        "NotifyVolumeDetached, # of subscribers: {}", subscribers.size());

    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if ((it->first == arrayName) || (it->first == ""))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeDetached to {}",
                it->second->Tag());
            it->second->VolumeDetached(volList, arrayName);

            POS_TRACE_DEBUG((int)POS_EVENT_ID::VOLUME_EVENT,
                "NotifyVolumeDetached to {} done",
                it->second->Tag());
        }
    }
}

} // namespace pos
