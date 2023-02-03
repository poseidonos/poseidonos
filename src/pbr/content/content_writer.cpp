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

#include "content_writer.h"
#include "i_content_serializer.h"
#include "content_serializer_factory.h"
#include "src/pbr/io/pbr_writer.h"

namespace pbr
{
ContentWriter::ContentWriter(uint32_t revision)
: ContentWriter(new PbrWriter(), revision)
{
}

ContentWriter::ContentWriter(IPbrWriter* writer, uint32_t revision)
: writer(writer)
{
    ContentSerializerFactory factory;
    serializer = factory.GetSerializer(revision);
}

ContentWriter::~ContentWriter(void)
{
    delete serializer;
    delete writer;
}

int
ContentWriter::Write(AteData* content, const pos::UblockSharedPtr dev)
{
    char* rawData = _Serialize(content);
    if (rawData == nullptr)
    {
        return -1;
    }
    int ret = writer->Write(dev, rawData, serializer->GetContentStartLba(), serializer->GetContentSize());
    delete rawData;
    return ret;
}

int
ContentWriter::Write(AteData* content, string filePath)
{
    char* rawData = _Serialize(content);
    if (rawData == nullptr)
    {
        return -1;
    }
    int ret = writer->Write(filePath, rawData, serializer->GetContentStartLba(), serializer->GetContentSize());
    delete rawData;
    return ret;
}

char*
ContentWriter::_Serialize(AteData* content)
{
    uint32_t length = serializer->GetContentSize();
    char* rawData = new char[length];
    int ret = serializer->Serialize(rawData, content);
    if (ret != 0)
    {
        delete rawData;
        return nullptr;
    }
    return rawData;
}

} // namespace pbr
