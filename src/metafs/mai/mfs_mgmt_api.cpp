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

#include "mfs_mgmt_api.h"

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsMgmtApiWrapperClass::Create(std::string& fileName, uint64_t fileByteSize, MetaFilePropertySet prop)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsMoMReqType::FileCreate;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = fileByteSize;
    reqMsg.fileProperty = prop;

    rc = mvm.HandleNewReq(reqMsg); // validity check & MetaVolMgrClass::_HandleCreateFileReq()

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsMgmtApiWrapperClass::Delete(std::string& fileName)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsMoMReqType::FileDelete;
    reqMsg.fileName = &fileName;
    rc = mvm.HandleNewReq(reqMsg);

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsMgmtApiWrapperClass::Open(std::string& fileName)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsMoMReqType::FileOpen;
    reqMsg.fileName = &fileName;
    rc = mvm.HandleNewReq(reqMsg); // validity check & MetaVolMgrClass::_HandleOpenFileReq()

    rc.returnData = reqMsg.completionData.openfd;
    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsMgmtApiWrapperClass::Close(uint32_t fd)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsMoMReqType::FileClose;
    reqMsg.fd = fd;
    rc = mvm.HandleNewReq(reqMsg); // validity check &  MetaVolMgrClass::_HandleCloseFileReq()

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsMgmtApiWrapperClass::CheckFileExist(std::string& fileName)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsMoMReqType::CheckFileExist;
    reqMsg.fileName = &fileName;
    rc = mvm.HandleNewReq(reqMsg);

    return rc;
}
