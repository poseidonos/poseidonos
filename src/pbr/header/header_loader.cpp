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

#include "header_loader.h"
#include "header_structure.h"
#include "src/pbr/io/pbr_reader.h"
#include "src/pbr/checker/header_checker.h"
#include "header_serializer.h"

namespace pbr
{
HeaderLoader::HeaderLoader(void)
: HeaderLoader(new PbrReader(), new HeaderChecker(), new HeaderSerializer())
{
}

HeaderLoader::HeaderLoader(IPbrReader* reader, IHeaderChecker* checker, IHeaderSerializer* serializer)
: reader(reader),
  checker(checker),
  serializer(serializer)
{
}

HeaderLoader::~HeaderLoader()
{
    delete serializer;
    delete checker;
    delete reader;
}

int
HeaderLoader::Load(HeaderElement* pHeaderOut, pos::UblockSharedPtr dev)
{
    uint64_t startLba = pbr::structure::header::START_LBA;
    uint32_t length = pbr::structure::header::LENGTH;

    char* rawData = new char[length];
    int ret = reader->Read(dev, rawData, startLba, length);
    if (ret == 0)
    {
        ret = serializer->Deserialize(rawData, length, pHeaderOut);
        if (ret == 0)
        {
            bool verified = checker->Check(pHeaderOut);
            if (verified == false)
            {
                ret = 1000;
            }
        }
    }
    delete rawData;
    return ret;
}

int
HeaderLoader::Load(HeaderElement* pHeaderOut, string filePath)
{
    int ret = 0;
    return ret;
}
} // namespace pbr
