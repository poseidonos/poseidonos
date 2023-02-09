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

#include "pbr_reader.h"
#include "src/device/base/ublock_device.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"

#include <fstream>

using namespace std;

namespace pbr
{
int
PbrReader::Read(pos::UblockSharedPtr dev, char* dataOut, uint64_t startLba, uint32_t length)
{
    uint32_t sectorSize = 512;
    uint32_t sectorCnt = length / sectorSize;
    if (length % sectorSize > 0)
    {
        sectorCnt++;
    }
    pos::UbioSmartPtr bio(new pos::Ubio(dataOut, sectorCnt, 0));
    bio->dir = pos::UbioDir::Read;
    bio->SetLba(startLba);
    bio->SetUblock(dev);
    pos::IODispatcherSingleton::Instance()->Submit(bio, true);
    return 0;
}

int
PbrReader::Read(string filePath, char* dataOut, uint64_t startOffset, uint32_t length)
{
    POS_TRACE_DEBUG(EID(PBR_LOAD_DEBUG), "load from file:{}", filePath);
    int ret = 0;
    std::ifstream checkFileExists(filePath);
    if(!checkFileExists.is_open())
    {
        ret = EID(PBR_FILE_NOT_EXIST);
        POS_TRACE_WARN(EID(PBR_FILE_NOT_EXIST), "file_path:{}", filePath);
    }
    else
    {
        ifstream f(filePath, ios::in | ios::ate);
        f.seekg(startOffset, ios::beg);
        f.read(dataOut, length);
        f.close();
    }
    return ret;
}
} // namespace pbr
