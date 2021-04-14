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

#include "metafs_management_api.h"

#include "msc_req.h"

namespace pos
{
MetaFsReturnCode<POS_EVENT_ID>
MetaFsManagementApi::MountSystem(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::MountSystem;
    reqMsg.arrayName = arrayName;
    rc.returnData = 0;
    rc = sysMgr.HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleMountReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsManagementApi::UnmountSystem(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::UnmountSystem;
    reqMsg.arrayName = arrayName;
    rc.returnData = 0;
    rc = sysMgr.HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleUnmountReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsManagementApi::CreateSystem(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::CreateSystem;
    reqMsg.arrayName = arrayName;
    rc.returnData = 0;
    rc = sysMgr.HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleFileSysCreateReq()

    return rc;
}

bool
MetaFsManagementApi::IsSystemMounted(void)
{
    return sysMgr.IsMounted();
}

bool
MetaFsManagementApi::AddArray(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::AddArray;
    reqMsg.arrayName = arrayName;
    rc = sysMgr.HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleAddArray()

    return rc.IsSuccess();
}

bool
MetaFsManagementApi::RemoveArray(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::RemoveArray;
    reqMsg.arrayName = arrayName;
    rc = sysMgr.HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleRemoveArray()

    return rc.IsSuccess();
}
} // namespace pos
