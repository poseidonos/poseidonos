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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

/* 
 * iBoFOS - Meta Filesystem Layer
 * 
 * Meta Filesystem Manager (metaFsMgr)
*/
#include "mfs_manager.h"

#if (0 == MFS_EXT_TESTDOUBLE_EN)
#include "instance_tagid_allocator.h"
#include "mfs_msc_top.h"
static InstanceTagIdAllocator aiocbTagIdAllocator;
#endif

bool
MetaFsMgrClass::Init(MetaStorageMediaInfoList& mediaInfoList)
{
#if (0 == MFS_EXT_TESTDOUBLE_EN)
    if (mediaInfoList.empty())
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_MODULE_NO_MEDIA,
            "No registered media info detected.");
        return false;
    }
    bool isSuccess = mscTopMgr.Init(mediaInfoList);
    if (!isSuccess)
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_MODULE_INIT_FAILED,
            "Failed to init. mscTopMgr");
    }

    isSuccess = mscTopMgr.Bringup();
    if (!isSuccess)
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_MODULE_BRINGUP_FAILED,
            "Failed to bringup mscTopMgr.");
    }
#endif
    return true;
}
