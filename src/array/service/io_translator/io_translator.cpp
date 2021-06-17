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

#include "io_translator.h"

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
IOTranslator::IOTranslator(void)
{
    translators = new ArrayTranslator[ArrayMgmtPolicy::MAX_ARRAY_CNT];
}

IOTranslator::~IOTranslator(void)
{
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        for (auto translator : translators[i])
        {
            delete translator.second;
        }
        translators[i].clear();
    }
}

bool
IOTranslator::Register(string array, ArrayTranslator trans)
{
    if (tempTranslators.find(array) == tempTranslators.end())
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::TRANSLATOR_DEBUG_MSG,
            "IOTranslator::Register, array:{} size:{}",
            array, trans.size());
        auto ret = tempTranslators.emplace(array, trans);
        return ret.second;
    }
    return true;
}

void
IOTranslator::Unregister(string array)
{
    _Erase(array);
}

int
IOTranslator::Translate(string array, PartitionType part,
    PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    ITranslator* trans = _Find(array, part);
    if (trans != nullptr)
    {
        return trans->Translate(dst, src);
    }

    int event = (int)POS_EVENT_ID::TRANSLATOR_NOT_EXIST;
    POS_TRACE_ERROR(event,
        "IOTranslator::Translate ERROR, array:{} part:{}", array, part);
    return event;
}

int
IOTranslator::Convert(string array, PartitionType part,
    list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src)
{
    ITranslator* trans = _Find(array, part);
    if (trans != nullptr)
    {
        return trans->Convert(dst, src);
    }

    int event = (int)POS_EVENT_ID::TRANSLATOR_NOT_EXIST;
    POS_TRACE_ERROR(event,
        "IOTranslator::Convert ERROR, array:{} part:{}", array, part);
    return event;
}

ITranslator*
IOTranslator::_Find(string array, PartitionType part)
{
    auto it = tempTranslators.find(array);
    if (it == tempTranslators.end())
    {
        return nullptr;
    }
    return it->second[part];
}

void
IOTranslator::_Erase(string array)
{
    if (array == "" && tempTranslators.size() == 1)
    {
        tempTranslators.clear();
    }
    else
    {
        tempTranslators.erase(array);
    }

    POS_TRACE_INFO((int)POS_EVENT_ID::TRANSLATOR_DEBUG_MSG,
        "IORecover::_Erase, array:{}, remaining:{}", array, tempTranslators.size());
}

// new iotranslator with arrayIndex

bool
IOTranslator::Register(unsigned int arrayIndex, ArrayTranslator trans)
{
    if (translators[arrayIndex].empty())
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::TRANSLATOR_DEBUG_MSG,
            "IOTranslator::Register, array:{} size:{}",
            arrayIndex, trans.size());
        translators[arrayIndex] = trans;
        return true;
    }
    return false;
}

void
IOTranslator::Unregister(unsigned int arrayIndex)
{
    translators[arrayIndex].clear();
}

int
IOTranslator::Translate(unsigned int arrayIndex, PartitionType part,
    PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        return it->second->Translate(dst, src);
    }

    int event = (int)POS_EVENT_ID::TRANSLATOR_NOT_EXIST;
    POS_TRACE_ERROR(event,
        "IOTranslator::Translate ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

int
IOTranslator::Convert(unsigned int arrayIndex, PartitionType part,
    list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src)
{
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        return it->second->Convert(dst, src);
    }

    int event = (int)POS_EVENT_ID::TRANSLATOR_NOT_EXIST;
    POS_TRACE_ERROR(event,
        "IOTranslator::Convert ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

} // namespace pos
