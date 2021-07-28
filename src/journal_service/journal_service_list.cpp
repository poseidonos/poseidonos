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

#include "journal_service_list.h"

#include "i_journal_manager.h"
#include "i_journal_status_provider.h"
#include "i_journal_writer.h"
#include "i_volume_event.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
template<typename T>
JournalServiceList<T>::JournalServiceList(void)
{
}

template<typename T>
JournalServiceList<T>::~JournalServiceList(void)
{
}

template<typename T>
T*
JournalServiceList<T>::Find(std::string name)
{
    // TODO (huijeong.kim) This's temporal workaround for current single array
    // Current Write handler in single array uses "" for arrayName
    if (name == "" && list.size() == 1)
    {
        return list.begin()->second;
    }

    auto found = list.find(name);
    if (found == list.end())
    {
        return nullptr;
    }
    else
    {
        return found->second;
    }
}

template<typename T>
void
JournalServiceList<T>::Register(std::string name, T* service)
{
    if (list.find(name) == list.end())
    {
        list.emplace(name, service);
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::JOURNAL_ALREADY_EXIST,
            "{} for array {} is already registered", typeid(T).name(), name);
    }
}

template<typename T>
void
JournalServiceList<T>::Unregister(std::string name)
{
    if (list.find(name) != list.end())
    {
        list.erase(name);
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::JOURNAL_ALREADY_EXIST,
            "{} for array {} already unregistered", typeid(T).name(), name);
    }
}

} // namespace pos
