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

#include "string_descripted_cpu_set_generator.h"

#include <algorithm>

#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
const char* StringDescriptedCpuSetGenerator::HEX_PREFIX = "0x";
const uint32_t StringDescriptedCpuSetGenerator::MAX_PROCESSABLE_CHAR = 16;

StringDescriptedCpuSetGenerator::StringDescriptedCpuSetGenerator(
    const CoreDescriptionArray& coreDescriptions, const bool forbidOverlap)
{
    CPU_ZERO(&overlapCheckCpuSet);
    for (auto& iter : coreDescriptions)
    {
        _SetCpuMaskFromString(iter.type, iter.coreRange, forbidOverlap);
    }
}

StringDescriptedCpuSetGenerator::~StringDescriptedCpuSetGenerator(void)
{
}

void
StringDescriptedCpuSetGenerator::_SetCpuMaskFromString(
    CoreType coreType, std::string cpuString, bool overlapCheck)
{
    bool isMask = (cpuString.find(HEX_PREFIX) == 0);
    if (isMask)
    {
        std::string hexOnlyString = cpuString.substr(strlen(HEX_PREFIX));
        _SetCpuMaskFromMask(coreType, hexOnlyString, overlapCheck);
    }
    else
    {
        _SetCpuMaskFromAllowedList(coreType, cpuString, overlapCheck);
    }
}

void
StringDescriptedCpuSetGenerator::_SetCpuMaskFromMask(
    CoreType coreType, std::string cpuString, bool overlapCheck)
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    std::string targetString = cpuString;
    uint32_t cipher = 0;

    while (targetString.empty() == false)
    {
        uint32_t stringSize = targetString.size();
        uint32_t targetSize = std::min(stringSize, MAX_PROCESSABLE_CHAR);
        uint32_t startOffset = stringSize - targetSize;
        std::string maskString = targetString.substr(startOffset, targetSize);
        uint64_t mask = stoull(maskString, nullptr, 16);

        for (uint64_t bit = 0; bit < BITS_PER_UINT64; bit++)
        {
            uint64_t maskingResult = mask & 1;
            if (maskingResult != 0)
            {
                uint32_t cpuIndex = cipher * BITS_PER_UINT64 + bit;
                bool isDuplicatedCpu = CPU_ISSET(cpuIndex, &overlapCheckCpuSet);
                bool isExceedTotalCpu = (cpuIndex > TOTAL_CORE_COUNT);
                if (isDuplicatedCpu || isExceedTotalCpu)
                {
                    POS_EVENT_ID eventId =
                        EID(AFTMGR_FAIL_TO_OVERLAP_MASK);
                    POS_TRACE_CRITICAL(static_cast<int>(eventId),
                        "Core mask is overlapped");
                    exit(-1);
                }
                else
                {
                    CPU_SET(cpuIndex, &cpuSet);
                    if (overlapCheck)
                    {
                        CPU_SET(cpuIndex, &overlapCheckCpuSet);
                    }
                }
            }
            mask >>= 1;
        }
        cipher++;
        uint32_t remainSize = stringSize - targetSize;
        targetString = targetString.substr(0, remainSize);
    }

    _SetCpuSet(coreType, cpuSet);
}

void
StringDescriptedCpuSetGenerator::_SetCpuMaskFromAllowedList(
    CoreType coreType, std::string cpuString, bool overlapCheck)
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    std::string targetString = cpuString;

    while (targetString.empty() == false)
    {
        size_t noneIntegerOffset = 0;
        uint32_t start = stoul(targetString, &noneIntegerOffset);
        uint32_t end = start;
        size_t nextOffset = noneIntegerOffset;

        if (targetString.size() > noneIntegerOffset)
        {
            char operand = targetString[noneIntegerOffset];
            nextOffset++;
            if (operand == '-')
            {
                targetString = targetString.substr(nextOffset);
                end = stoul(targetString, &noneIntegerOffset);
                nextOffset = noneIntegerOffset + 1;
            }
        }
        _SetCpuRange(start, end, overlapCheck, &cpuSet);
        if (nextOffset > targetString.size())
        {
            break;
        }
        targetString = targetString.substr(nextOffset);
    }

    _SetCpuSet(coreType, cpuSet);
}

void
StringDescriptedCpuSetGenerator::_SetCpuRange(
    uint32_t from, uint32_t to, bool overlapCheck, cpu_set_t* prevCpuSet)
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (uint32_t cpu = from; cpu <= to; cpu++)
    {
        CPU_SET(cpu, &cpuSet);
    }
    cpu_set_t cpuSetAnd;
    CPU_AND(&cpuSetAnd, &cpuSet, &overlapCheckCpuSet);
    uint32_t cpuCount = CPU_COUNT(&cpuSetAnd);
    bool isExceed = (to >= TOTAL_CORE_COUNT);
    bool isOverlaped = (cpuCount > 0);
    if (isExceed || isOverlaped)
    {
        POS_EVENT_ID eventId =
            EID(AFTMGR_FAIL_TO_PARSING_ERROR);
        POS_TRACE_CRITICAL(static_cast<int>(eventId),
            "Cpu allowed list is wrongly set");
        exit(-1);
    }

    if (overlapCheck)
    {
        CPU_OR(&overlapCheckCpuSet, &overlapCheckCpuSet, &cpuSet);
    }

    CPU_OR(prevCpuSet, prevCpuSet, &cpuSet);
}

} // namespace pos
