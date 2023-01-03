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

#include "header_serializer.h"
#include "header_structure.h"
#include <string.h>
#include <string>
#include <iostream>

namespace pbr
{

int
HeaderSerializer::Serialize(HeaderElement* pHeader, char* dataOut, uint32_t length)
{
    using namespace pbr::structure::header;
    {
        char* signature = &dataOut[SIGNATURE_OFFSET];
        strncpy(signature, pHeader->signature.c_str(), SIGNATURE_LENGTH);
    }
    {
        string tmp = to_string(pHeader->revision);
        char* revision = &dataOut[REVISION_OFFSET];
        strncpy(revision, tmp.c_str(), REVISION_LENGTH);
    }
    {
        string tmp = to_string(pHeader->checksum);
        char* checksum = &dataOut[CHECKSUM_OFFSET];
        strncpy(checksum, tmp.c_str(), CHECKSUM_LENGTH);
    }

    return 0;
}

int
HeaderSerializer::Deserialize(char* rawData, uint32_t length, HeaderElement* pHeaderOut)
{
    using namespace pbr::structure::header;
    {
        char signature[SIGNATURE_LENGTH] = {'\0',};
        memcpy(signature, &rawData[SIGNATURE_OFFSET], SIGNATURE_LENGTH);
        pHeaderOut->signature = signature;
    }
    {
        char revision[REVISION_LENGTH] = {'\0',};
        memcpy(revision, &rawData[REVISION_OFFSET], REVISION_LENGTH);
        pHeaderOut->revision = atoi(revision);
    }
    {
        char checksum[CHECKSUM_LENGTH] = {'\0',};
        memcpy(checksum, &rawData[CHECKSUM_OFFSET], CHECKSUM_LENGTH);
        pHeaderOut->checksum = atoi(checksum);
    }

    return 0;
}

} // namespace pbr
