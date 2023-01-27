/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "directive_cmd.h"

#include "src/bio/ubio.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"

namespace pos
{
DirectiveContext::DirectiveContext(bool cmd, uint32_t doper, uint32_t dtype, uint32_t dspec,
        void* payload, uint32_t payloadSize, uint32_t cdw12, uint32_t cdw13)
: cmd(cmd),
  doper(doper),
  dtype(dtype),
  dspec(dspec),
  payload(payload),
  payloadSize(payloadSize),
  cdw12(cdw12),
  cdw13(cdw13)
{
}

DirectiveCmd::DirectiveCmd(UblockSharedPtr dev, spdk_nvme_ctrlr* ctrlr, IIODispatcher* ioDispatcher)
: targetDevice(dev),
  ctrlr(ctrlr),
  ioDispatcher(ioDispatcher)
{
}

DirectiveCmd::~DirectiveCmd(void)
{
}

void
DirectiveCmd::EnableDirective(void)
{
    _SubmitUbio(DirReqId::IDENTIFY_DIRECTIVE_SEND_ENABLED);
}

void
DirectiveCmd::ReturnIdentifyParameters(void)
{
    _SubmitUbio(DirReqId::IDENTIFY_DIRECTIVE_RECEIVE_RETURN_PARAM);
}

void
DirectiveCmd::AllocateResources(void)
{
    _SubmitUbio(DirReqId::STREAMS_DIRECTIVE_RECEIVE_ALLOCATE_RESOURCE);
}

void
DirectiveCmd::ReleaseResources(void)
{
    _SubmitUbio(DirReqId::STREAMS_DIRECTIVE_SEND_RELEASE_RESOURCE);
}

void
DirectiveCmd::_SubmitUbio(DirReqId reqId)
{
    uint32_t payloadSize = 0;
    void* payload = nullptr;
    DirectiveContext* directiveContext = nullptr;
    switch (reqId)
    {
        case DirReqId::IDENTIFY_DIRECTIVE_SEND_ENABLED:
            directiveContext = new DirectiveContext(
                DIRECTIVE_SEND,
                SPDK_NVME_IDENTIFY_DIRECTIVE_SEND_ENABLED,
                SPDK_NVME_DIRECTIVE_TYPE_IDENTIFY,
                0,
                payload,
                payloadSize,
                ENABLE_DIRECTIVE,
                0);
            break;

        case DirReqId::IDENTIFY_DIRECTIVE_RECEIVE_RETURN_PARAM:
            payloadSize = sizeof(spdk_nvme_ns_identify_directive_param);
            payload = new struct spdk_nvme_ns_identify_directive_param();
            directiveContext = new DirectiveContext(
                DIRECTIVE_RECEIVE,
                SPDK_NVME_IDENTIFY_DIRECTIVE_RECEIVE_RETURN_PARAM,
                SPDK_NVME_DIRECTIVE_TYPE_IDENTIFY,
                0,
                payload,
                payloadSize,
                0,
                0);
            break;

        case DirReqId::STREAMS_DIRECTIVE_RECEIVE_ALLOCATE_RESOURCE:
            directiveContext = new DirectiveContext(
                DIRECTIVE_RECEIVE,
                SPDK_NVME_STREAMS_DIRECTIVE_RECEIVE_ALLOCATE_RESOURCE,
                SPDK_NVME_DIRECTIVE_TYPE_STREAMS,
                0,
                payload,
                payloadSize,
                STREAM_CNT,
                0);
            break;

        case DirReqId::STREAMS_DIRECTIVE_SEND_RELEASE_RESOURCE:
            directiveContext = new DirectiveContext(
                DIRECTIVE_SEND,
                SPDK_NVME_STREAMS_DIRECTIVE_SEND_RELEASE_RESOURCE,
                SPDK_NVME_DIRECTIVE_TYPE_STREAMS,
                0,
                payload,
                payloadSize,
                0,
                0);
            break;

        default:
            return;
    }

    if (0 == payloadSize)
    {
        payloadSize = 1;
    }
    UbioSmartPtr ubio(new Ubio((void*)directiveContext, payloadSize, 0 /*array index*/));
    ubio->dir = UbioDir::Directive;
    ubio->SetUblock(targetDevice);

    int ret = ioDispatcher->Submit(ubio, true);
    if (ret < 0 || ubio->GetError() != IOErrorType::SUCCESS)
    {
        switch (reqId)
        {
            case DirReqId::IDENTIFY_DIRECTIVE_SEND_ENABLED:
                POS_TRACE_INFO(EID(UNVME_OPERATION_NOT_SUPPORTED),
                    "Failed to Enable directives. result:{} ubio->GetError():{}",
                    ret, ubio->GetError());
                break;

            case DirReqId::STREAMS_DIRECTIVE_RECEIVE_ALLOCATE_RESOURCE:
                POS_TRACE_INFO(EID(UNVME_OPERATION_NOT_SUPPORTED),
                    "Failed to allocate resources. result:{} ubio->GetError():{}",
                    ret, ubio->GetError());
                break;

            default:
                POS_TRACE_INFO(EID(UNVME_OPERATION_NOT_SUPPORTED),
                    "Directive Command Failed. result:{} ubio->GetError():{}",
                    ret, ubio->GetError());
        }
    }
}
} // namespace pos
