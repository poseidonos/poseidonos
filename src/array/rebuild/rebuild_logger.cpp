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

#include "rebuild_logger.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"
#include "src/helper/time/time_helper.h"

#include <fstream>
#include <iostream>
#include <iomanip>

namespace pos
{

RebuildLogger::RebuildLogger(string arrayName, string type)
: array(arrayName),
  type(type)
{
}

// LCOV_EXCL_START
RebuildLogger::~RebuildLogger(void)
{
}
// LCOV_EXCL_STOP

void RebuildLogger::SetArrayRebuildStart(void)
{
    start = chrono::system_clock::now();
}

void RebuildLogger::SetPartitionRebuildStart(string partName)
{
    partStart.emplace(partName, chrono::system_clock::now());
}

void RebuildLogger::SetResult(string result)
{
    end = chrono::system_clock::now();
    rebuildResult = result;
}

void RebuildLogger::WriteLog(void)
{
    chrono::duration<double> duration = end - start;
    uint64_t mb = 1024 * 1024;
    uint64_t restoredSizeMB = rebuiltSegCnt * ArrayConfig::SSD_SEGMENT_SIZE_BYTE / mb;
    string logDir = LoggerSingleton::Instance()->GetLogDir();
    string fileName = "rebuild_log";
    ofstream ofile;
    ofile.open(logDir + fileName, ios_base::app);
    if (ofile.is_open())
    {
        ofile << "=======Rebuild Result=======" << endl;
        ofile << "Name: " << array <<endl;
        ofile << "Type: " << type <<endl;
        ofile << "Start: " << Time::ToString(chrono::system_clock::to_time_t(start)) << endl;
        for (auto it : partStart)
        {
            ofile << it.first << " rebuild starts: " << Time::ToString(chrono::system_clock::to_time_t(it.second)) <<endl;
        }
        ofile << "End: " << Time::ToString(chrono::system_clock::to_time_t(end)) <<endl;
        ofile << "Duration: " << duration.count() << " (s)"  <<endl;
        ofile << "Result: " << rebuildResult <<endl;
        ofile << "Number of rebuilt segments: " << rebuiltSegCnt <<endl;
        ofile << "Total restoration size: " << restoredSizeMB << " MB" << endl;
        ofile << "Restoration Speed: " << (double)restoredSizeMB / (double)(duration.count()) << " MB/s" <<endl;
        ofile << "============================" << endl;
        ofile.close();
    }
}

} // namespace pos
