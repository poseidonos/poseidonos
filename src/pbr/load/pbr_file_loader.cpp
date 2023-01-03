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

#include "pbr_file_loader.h"
#include "src/pbr/header/header_loader.h"
#include "src/pbr/load/pbr_selector.h"
#include "src/pbr/content/content_loader.h"
#include "src/pbr/content/content_serializer_factory.h"

namespace pbr
{
PbrFileLoader::PbrFileLoader(vector<string> fileList)
: PbrFileLoader(new HeaderLoader(), new PbrSelector(), fileList)
{
}

PbrFileLoader::PbrFileLoader(IHeaderLoader* headerLoader, IPbrSelector* pbrSelector,
    vector<string> fileList)
: headerLoader(headerLoader),
  pbrSelector(pbrSelector),
  fileList(fileList)
{
}

PbrFileLoader::~PbrFileLoader(void)
{
    delete pbrSelector;
    delete headerLoader;
}

int
PbrFileLoader::Load(vector<AteData*>& out)
{
    int ret = 0;
    ContentSerializerFactory factory;
    for (auto filePath : fileList)
    {
        HeaderElement header;
        ret = headerLoader->Load(&header, filePath);
        if (ret == 0)
        {
            AteData* ateData = new AteData();
            ContentLoader contentLoader(factory.GetSerializer(header.revision));
            ret = contentLoader.Load(ateData, filePath);
            if (ret == 0)
            {
                out.push_back(ateData);
            }
            else
            {
                delete ateData;
            }
        }
    }
    if (ret == 0)
    {
        ret = pbrSelector->Select(out);
    }
    return ret;
}
} // namespace pbr
