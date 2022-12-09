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
MDPage::ClearCtrlInfo(void)
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
MDPage::Make(const MetaLpnType metaLpn, const FileDescriptorType fd,
    const int arrayId, const uint64_t signature)
{
    ctrlInfo->mfsSignature = MDPageControlInfo::MDPAGE_CTRL_INFO_SIG;
    ctrlInfo->epochSignature = signature;
    ctrlInfo->metaLpn = metaLpn;
    ctrlInfo->fd = fd;
}

bool
MDPage::CheckValid(const int arrayId, const uint64_t signature) const
{
    // detect mdpage validity by combining two signatures
    // note that it has to have additional logic to detect signature corruption case later on
    if (ctrlInfo->mfsSignature != MDPageControlInfo::MDPAGE_CTRL_INFO_SIG)
    {
        MFS_TRACE_DEBUG(EID(MFS_INVALID_PARAMETER),
            "The mdpage signature in the control is invalid, sig: {}",
            ctrlInfo->mfsSignature);

        return false;
    }

    if (ctrlInfo->epochSignature != signature)
    {
        MFS_TRACE_DEBUG(EID(MFS_INVALID_PARAMETER),
            "The epoch signature in the control is invalid, ideal sig: {}, sig: {}",
            signature, ctrlInfo->epochSignature);

        return false;
    }

    return true;
}

bool
MDPage::CheckLpnMismatch(const MetaLpnType srcLpn) const
{
    if (ctrlInfo->metaLpn != srcLpn)
    {
        POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "Lpn mismatch detected: ideal lpn: {}, lpn: {}",
            srcLpn, ctrlInfo->metaLpn);

        return false;
    }
    return true;
}

bool
MDPage::CheckFileMismatch(const FileDescriptorType fd) const
{
    if (ctrlInfo->fd != fd)
    {
        POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "FD mismatch detected: ideal fd: {}, fd: {}, lpn: {}",
            fd, ctrlInfo->fd, ctrlInfo->metaLpn);
        return false;
    }
    return true;
}
} // namespace pos
