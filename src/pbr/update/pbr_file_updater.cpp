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

#include "pbr_file_updater.h"
#include "src/pbr/header/header_serializer.h"
#include "src/pbr/content/content_serializer_factory.h"
#include "src/pbr/io/pbr_writer.h"
#include "src/logger/logger.h"

namespace pbr
{
PbrFileUpdater::PbrFileUpdater(uint32_t revision, string filePath)
: PbrFileUpdater(new HeaderSerializer(), ContentSerializerFactory::GetSerializer(revision),
    new PbrWriter(), revision, filePath)
{
}

PbrFileUpdater::PbrFileUpdater(IHeaderSerializer* headerSerializer, IContentSerializer* contentSerializer,
    IPbrWriter* pbrWriter, uint32_t revision, string filePath)
: headerSerializer(headerSerializer),
  contentSerializer(contentSerializer),
  pbrWriter(pbrWriter),
  revision(revision),
  filePath(filePath)
{
}

PbrFileUpdater::~PbrFileUpdater(void)
{
    delete pbrWriter;
    delete contentSerializer;
    delete headerSerializer;
}

int
PbrFileUpdater::Update(AteData* ateData)
{
    HeaderElement headerElem { revision };
    uint32_t pbrSize = header::LENGTH + contentSerializer->GetContentSize();
    char* pbrData = new char[pbrSize];
    memset(pbrData, 0, pbrSize);
    int ret = headerSerializer->Serialize(&headerElem, pbrData, header::LENGTH);
    if (ret == 0)
    {
        uint32_t contentOffset = contentSerializer->GetContentStartLba();
        ret = contentSerializer->Serialize(&pbrData[contentOffset], ateData);
        if (ret == 0)
        {
            pbrWriter->Write(filePath, pbrData, 0, pbrSize);
        }
    }
    delete[] pbrData;
    return ret;
}

int
PbrFileUpdater::Clear(void)
{
    return remove(filePath.c_str());
}
} // namespace pbr
