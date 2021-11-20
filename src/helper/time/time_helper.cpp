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

#include "time_helper.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace std;

string
GetCurrentTimeStr(string timeFormat, int maxBufferSize)
{
    time_t currentTime = time(0);
    struct tm timeStruct;
    char* timeBuf = new char[maxBufferSize];
    localtime_r(&currentTime, &timeStruct);
    strftime(timeBuf, maxBufferSize, timeFormat.c_str(), &timeStruct);
    string result(timeBuf);
    delete[] timeBuf;
    return result;
}

string
GetCurrentTimeStr(string timeFormat)
{
    int dateTimeSize = 32;
    return GetCurrentTimeStr(timeFormat, dateTimeSize);
}

time_t
GetTimeT(string datetime, string timeFormat)
{
    time_t currentTime = time(0);
    time_t ret;
    struct tm timeStruct;
    localtime_r(&currentTime, &timeStruct); // timezone bug bypass
    strptime(datetime.c_str(), timeFormat.c_str(), &timeStruct);
    ret = mktime(&timeStruct);
    if (ret == -1)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_TIME_CALC_ERROR,
            "Time calculation error");
    }
    return ret;
}
