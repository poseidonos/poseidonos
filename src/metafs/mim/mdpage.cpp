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

#include "mdpage.h"

#include <boost/crc.hpp>

#include "mdpage_buf_pool.h"
#include "meta_io_manager.h"
#include "metafs_config.h"
#include "metafs_log.h"
#include "os_header.h"
#include "src/include/memory.h"

namespace pos
{
MDPage::MDPage(void* buf)
: dataAll(reinterpret_cast<uint8_t*>(buf)),
  ctrlInfo(nullptr)
{
}

MDPage::~MDPage(void)
{
}

void
MDPage::ClearControlInfo(void)
{
    // If there is an error when issuing a read command, the ctrlInfo is null.
    if (ctrlInfo)
    {
        memset(ctrlInfo, 0x0, sizeof(MDPageControlInfo));
    }
}

void
MDPage::AttachControlInfo(void)
{
    if (!ctrlInfo)
    {
        ctrlInfo = reinterpret_cast<MDPageControlInfo*>(dataAll + GetDefaultDataChunkSize());
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "[Mpio][ControlInfo] Control info attached");
    }
}

void
MDPage::BuildControlInfo(const MetaLpnType metaLpn, const FileDescriptorType fd,
    const int arrayId, const uint64_t signature)
{
    ctrlInfo->mfsSignature = MDPageControlInfo::MDPAGE_CTRL_INFO_SIG;
    ctrlInfo->epochSignature = signature;
    ctrlInfo->metaLpn = metaLpn;
    ctrlInfo->fd = fd;
    ctrlInfo->crc = _GenerateCrc();
}

bool
MDPage::IsValidSignature(const uint64_t signature) const
{
    // detect mdpage validity by combining two signatures
    // note that it has to have additional logic to detect signature corruption case later on
    if (ctrlInfo->mfsSignature != MDPageControlInfo::MDPAGE_CTRL_INFO_SIG)
    {
        MFS_TRACE_DEBUG(EID(MFS_INVALID_SIGNATURE),
            "The mdpage signature in the control is invalid, {}",
            _ToString());

        return false;
    }

    if (ctrlInfo->epochSignature != signature)
    {
        MFS_TRACE_DEBUG(EID(MFS_INVALID_EPOCH_SIGNATURE),
            "The epoch signature in the control is invalid, ideal epochSignature: {}, {}",
            signature, _ToString());

        return false;
    }

    return true;
}

bool
MDPage::_DoesMetaLpnMatch(const MetaLpnType srcLpn) const
{
    if (ctrlInfo->metaLpn != srcLpn)
    {
        return false;
    }
    return true;
}

bool
MDPage::_DoesFileDescriptorMatch(const FileDescriptorType fd) const
{
    if (ctrlInfo->fd != fd)
    {
        return false;
    }
    return true;
}

int
MDPage::CheckDataIntegrity(const MetaLpnType srcLpn, const FileDescriptorType fd,
    const bool skipCheckingCrc) const
{
    if (!_DoesMetaLpnMatch(srcLpn))
    {
        int result = EID(MFS_INVALID_META_LPN);
        POS_TRACE_ERROR(result,
            "Lpn mismatch detected: ideal lpn: {}, {}",
            srcLpn, _ToString());
        return result;
    }

    if (!_DoesFileDescriptorMatch(fd))
    {
        int result = EID(MFS_INVALID_FILE_DESCRIPTOR);
        POS_TRACE_ERROR(result,
            "FD mismatch detected: ideal fd: {}, {}",
            fd, ctrlInfo->fd, _ToString());
        return result;
    }

    if (!skipCheckingCrc)
    {
        uint32_t actualCrc = _GenerateCrc();
        if (ctrlInfo->crc != actualCrc)
        {
            int result = EID(MFS_INVALID_CRC);
            POS_TRACE_ERROR(result,
                "Crc mismatch detected, crc generated: {}, {}",
                actualCrc, _ToString());
            return result;
        }
    }

    return EID(SUCCESS);
}

uint32_t
MDPage::GenerateCrcFromDataBuffer(void) const
{
    return _GenerateCrc();
}

uint32_t
MDPage::_GenerateCrc(void) const
{
    boost::crc_32_type result;
    result.reset();
    result.process_bytes(dataAll, GetCrcCoveredSize());
    return result.checksum();
}

std::string
MDPage::_ToString(void) const
{
    std::string str("data from nvm ");
    str = str.append("mfsSignature: " + std::to_string(ctrlInfo->mfsSignature));
    str = str.append(", epochSignature: " + std::to_string(ctrlInfo->epochSignature));
    str = str.append(", version: " + std::to_string(ctrlInfo->version));
    str = str.append(", fd: " + std::to_string(ctrlInfo->fd));
    str = str.append(", metaLpn: " + std::to_string(ctrlInfo->metaLpn));
    str = str.append(", crc: " + std::to_string(ctrlInfo->crc));
    return str;
}
} // namespace pos
