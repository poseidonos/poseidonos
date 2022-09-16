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

#include "io_translator.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
IOTranslator::IOTranslator(void)
{
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
IOTranslator::Register(unsigned int arrayIndex, ArrayTranslator trans)
{
    if (translators[arrayIndex].empty())
    {
        int eventId = EID(ADDRESS_TRANSLATE_DEBUG_MSG);
        if (trans.empty())
        {
            POS_TRACE_WARN(eventId,
                "IOTranslator::Register, no translator exists, array:{} size:{}",
                arrayIndex, trans.size());
        }
        POS_TRACE_INFO(eventId,
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
    list<PhysicalEntry>& pel, const LogicalEntry& le)
{
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        return it->second->Translate(pel, le);
    }

    int event = EID(IO_TRANSLATOR_NOT_FOUND);
    POS_TRACE_ERROR(event,
        "IOTranslator::Translate ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

int
IOTranslator::ByteTranslate(unsigned int arrayIndex, PartitionType part,
    PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    int event;
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        if (it->second->IsByteAccessSupported())
        {
            return it->second->ByteTranslate(dst, src);
        }
        else
        {
            event = EID(ADDRESS_BYTE_TRANSLATION_IS_NOT_SUPPORTED);
            POS_TRACE_ERROR(event,
                "IOTranslator::ByteTranslate not supported, array:{} part:{}", arrayIndex, part);
            return event;
        }
    }

    event = EID(ADDRESS_TRANSLATE_DEBUG_MSG);
    POS_TRACE_ERROR(event,
        "IOTranslator::Translate ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

int
IOTranslator::GetParityList(unsigned int arrayIndex, PartitionType part,
    list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src)
{
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        return it->second->GetParityList(parity, src);
    }

    int event = EID(ADDRESS_TRANSLATE_DEBUG_MSG);
    POS_TRACE_ERROR(event,
        "IOTranslator::Convert ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

int
IOTranslator::ByteConvert(unsigned int arrayIndex, PartitionType part,
    list<PhysicalByteWriteEntry>& dst, const LogicalByteWriteEntry& src)
{
    int event;
    auto it = translators[arrayIndex].find(part);
    if (it != translators[arrayIndex].end())
    {
        if (it->second->IsByteAccessSupported())
        {
            return it->second->ByteConvert(dst, src);
        }
        else
        {
            event = EID(ADDRESS_BYTE_TRANSLATION_IS_NOT_SUPPORTED);
            POS_TRACE_ERROR(event,
                "IOTranslator::ByteTranslate not supported, array:{} part:{}", arrayIndex, part);
            return event;
        }
    }

    event = EID(ADDRESS_TRANSLATE_DEBUG_MSG);
    POS_TRACE_ERROR(event,
        "IOTranslator::Convert ERROR, array:{} part:{}", arrayIndex, part);
    return event;
}

} // namespace pos
