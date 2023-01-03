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

#include "content_loader.h"
#include "src/pbr/checker/content_checker.h"
#include "src/pbr/io/pbr_reader.h"
#include <memory.h>

namespace pbr
{
ContentLoader::ContentLoader(IContentSerializer* serializer)
: ContentLoader(new PbrReader(), new ContentChecker(), serializer)
{
}

ContentLoader::ContentLoader(IPbrReader* reader, IContentChecker* checker,
    IContentSerializer* serializer)
: reader(reader),
  checker(checker),
  serializer(serializer)
{
}

ContentLoader::~ContentLoader()
{
    delete serializer;
    delete checker;
    delete reader;
}

int
ContentLoader::Load(AteData* out, pos::UblockSharedPtr dev)
{
    uint32_t length = serializer->GetContentSize();
    uint64_t startLba = serializer->GetContentStartLba();
    char* rawData = new char[length];
    memset(rawData, 0, length);
    int ret = reader->Read(dev, rawData, startLba, length);
    if (ret == 0)
    {
        ret = serializer->Deserialize(out, rawData);
        if (ret == 0)
        {
            bool verified = checker->Check(out);
            if (verified == false)
            {
                ret = 1000;
            }
        }
    }
    delete[] rawData;
    return ret;
}

int
ContentLoader::Load(AteData* out, string filePath)
{
    uint32_t length = serializer->GetContentSize();
    uint64_t startOffset = serializer->GetContentStartLba();
    char* rawData = new char[length];
    memset(rawData, 0, length);
    int ret = reader->Read(filePath, rawData, startOffset, length);
    if (ret == 0)
    {
        ret = serializer->Deserialize(out, rawData);
        if (ret == 0)
        {
            bool verified = checker->Check(out);
            if (verified == false)
            {
                ret = 1000;
            }
        }
    }
    delete[] rawData;
    return ret;
}

} // namespace pbr
