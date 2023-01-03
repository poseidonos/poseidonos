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

#include "pbr_updater.h"
#include "src/pbr/header/header_writer.h"
#include "src/pbr/content/content_writer.h"

namespace pbr
{
PbrUpdater::PbrUpdater(uint32_t revision, vector<pos::UblockSharedPtr> devs)
: PbrUpdater(new HeaderWriter(), new ContentWriter(revision), revision, devs)
{
}

PbrUpdater::PbrUpdater(IHeaderWriter* headerWriter, IContentWriter* contentWriter,
    uint32_t revision, vector<pos::UblockSharedPtr> devs)
: headerWriter(headerWriter),
  contentWriter(contentWriter),
  revision(revision),
  devs(devs)
{
}

PbrUpdater::~PbrUpdater(void)
{
    delete contentWriter;
    delete headerWriter;
}

int
PbrUpdater::Update(AteData* ateData)
{
    HeaderElement header{"PBR", revision, 0};
    for (auto dev : devs)
    {
        int ret = headerWriter->Write(&header, dev);
        if (ret == 0)
        {
            ret = contentWriter->Write(ateData, dev);
        }
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

int
PbrUpdater::Clear(void)
{
    for (auto dev : devs)
    {
        HeaderElement header {"", 0, 0};
        int ret = headerWriter->Write(&header, dev);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}
} // namespace pbr
