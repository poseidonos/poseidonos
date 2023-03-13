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

#pragma once

#include "src/pbr/interface/i_pbr_updater.h"
#include "src/pbr/header/i_header_serializer.h"
#include "src/pbr/content/i_content_serializer.h"
#include "src/pbr/io/i_pbr_writer.h"

#include <vector>
#include <memory>

using namespace std;

namespace pbr
{
class PbrFileUpdater : public IPbrUpdater
{
public:
    PbrFileUpdater(uint32_t revision, string filePath);
    PbrFileUpdater(unique_ptr<IHeaderSerializer> headerSerializer,
        unique_ptr<IContentSerializer> contentSerializer,
        unique_ptr<IPbrWriter> pbrWriter,
        uint32_t revision,
        string filePath);
    virtual ~PbrFileUpdater();

protected:
    virtual int Update(AteData* ateData) override;
    virtual int Clear(void) override;

private:
    unique_ptr<IHeaderSerializer> headerSerializer = nullptr;
    unique_ptr<IContentSerializer> contentSerializer = nullptr;
    unique_ptr<IPbrWriter> pbrWriter = nullptr;
    uint32_t revision;
    string filePath;
};
} // namespace pbr
