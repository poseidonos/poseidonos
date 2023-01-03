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

#include "header_writer.h"
#include "header_structure.h"
#include "header_serializer.h"
#include "src/pbr/checker/header_checker.h"
#include "src/pbr/io/pbr_writer.h"

namespace pbr
{
HeaderWriter::HeaderWriter(void)
: HeaderWriter(new PbrWriter(), new HeaderChecker(), new HeaderSerializer())
{
}

HeaderWriter::HeaderWriter(IPbrWriter* writer, IHeaderChecker* checker, IHeaderSerializer* serializer)
: writer(writer),
  checker(checker),
  serializer(serializer)
{
}

HeaderWriter::~HeaderWriter(void)
{
    delete serializer;
    delete checker;
    delete writer;
}

int
HeaderWriter::Write(HeaderElement* pHeader, pos::UblockSharedPtr dev)
{
    int ret = checker->UpdateChecksum(pHeader);
    if (ret == 0)
    {
        uint64_t startLba = pbr::structure::header::START_LBA;
        uint32_t length = pbr::structure::header::LENGTH;
        char* rawData = new char[length];
        int ret = serializer->Serialize(pHeader, rawData, length);
        if (ret == 0)
        {
            writer->Write(dev, rawData, startLba, length);
        }
        delete rawData;
    }
    return ret;
}

int
HeaderWriter::Write(HeaderElement* pHeader, string filePath)
{
    int ret = checker->UpdateChecksum(pHeader);
    if (ret == 0)
    {
        uint64_t startLba = pbr::structure::header::START_LBA;
        uint32_t length = pbr::structure::header::LENGTH;
        char* rawData = new char[length] {'\0',};
        int ret = serializer->Serialize(pHeader, rawData, length);
        if (ret == 0)
        {
            writer->Write(filePath, rawData, startLba, length);
        }
        delete rawData;
    }
    return ret;
}
} // namespace pbr
