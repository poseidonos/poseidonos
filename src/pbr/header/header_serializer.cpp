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
#include "src/pbr/checker/pbr_checksum.h"
#include "src/helper/string/hex_string_converter.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <string.h>
#include <string>
#include <iostream>

namespace pbr
{

int
HeaderSerializer::Serialize(HeaderElement* headerElem, char* dataOut, uint32_t length)
{
    memset(dataOut, 0, length);
    strncpy(&dataOut[header::SIGNATURE_OFFSET], headerElem->signature.c_str(), header::SIGNATURE_LENGTH);
    uint32_to_hex(headerElem->revision, &dataOut[header::REVISION_OFFSET], header::REVISION_LENGTH);
    uint32_t checksum = MakePbrChecksum(dataOut, length, header::CHECKSUM_OFFSET, header::CHECKSUM_LENGTH);
    uint32_to_hex(checksum, &dataOut[header::CHECKSUM_OFFSET], header::CHECKSUM_LENGTH);
    return 0;
}

int
HeaderSerializer::Deserialize(char* rawData, uint32_t length, HeaderElement* headerElemOut)
{
    { // PBR header signature verification
        char signature[header::SIGNATURE_LENGTH + 1] = {'\0',};
        memcpy(signature, &rawData[header::SIGNATURE_OFFSET], header::SIGNATURE_LENGTH);
        if (signature != header::SIGNATURE)
        {
            int eid = EID(PBR_UNKNOWN_SIGNATURE);
            POS_TRACE_WARN(eid, "{}", signature);
            return eid;
        }
        headerElemOut->signature = signature;
    } ////
    { // checksum verification
        uint32_t actual = hex_to_uint32(&rawData[header::CHECKSUM_OFFSET], header::CHECKSUM_LENGTH);
        uint32_t expected = MakePbrChecksum(rawData, length,
            header::CHECKSUM_OFFSET, header::CHECKSUM_LENGTH);
        if (actual != expected)
        {
            int eid = EID(PBR_CHECKSUM_INVALID);
            POS_TRACE_WARN(eid, "actual_value:{}, expected_value:{}", actual, expected);
            return eid;
        }
        else
        {
            POS_TRACE_DEBUG(EID(PBR_CHECKSUM_VALID), "value:{}", actual);
        }
    } ////
    {
        char revision[header::REVISION_LENGTH + 1] = {'\0',};
        memcpy(revision, &rawData[header::REVISION_OFFSET], header::REVISION_LENGTH);
        headerElemOut->revision = atoi(revision);
    }
    return 0;
}

} // namespace pbr
