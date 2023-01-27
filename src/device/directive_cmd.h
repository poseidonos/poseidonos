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

#ifndef DIRECTIVE_CMD_H_
#define DIRECTIVE_CMD_H_

#include "spdk/include/spdk/nvme.h"
#include "spdk/include/spdk/nvme_spec.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/spdk_wrapper/caller/spdk_nvme_caller.h"

enum spdk_nvme_directive_type;

enum spdk_nvme_identify_directive_send_operation;
enum spdk_nvme_identify_directive_receive_operation;
struct spdk_nvme_ns_identify_directive_param;

enum spdk_nvme_streams_directive_receive_operation;
enum spdk_nvme_streams_directive_send_operation;

namespace pos
{
class UBlockDevice;
class IIODispatcher;

enum DirectiveCommmand
{
    DIRECTIVE_RECEIVE = 0,
    DIRECTIVE_SEND
};

enum class DirReqId
{
    IDENTIFY_DIRECTIVE_SEND_ENABLED = 0,
    IDENTIFY_DIRECTIVE_RECEIVE_RETURN_PARAM,
    STREAMS_DIRECTIVE_RECEIVE_RETURN_PARAM,
    STREAMS_DIRECTIVE_RECEIVE_GET_STATUS,
    STREAMS_DIRECTIVE_RECEIVE_ALLOCATE_RESOURCE,
    STREAMS_DIRECTIVE_SEND_RELEASE_ID,
    STREAMS_DIRECTIVE_SEND_RELEASE_RESOURCE
};

class DirectiveContext
{
public:
    DirectiveContext(bool cmd, uint32_t doper, uint32_t dtype, uint32_t dspec,
        void* payload, uint32_t payloadSize, uint32_t cdw12, uint32_t cdw13);
    bool cmd;
    uint32_t doper;
    uint32_t dtype;
    uint32_t dspec;
    void* payload;
    uint32_t payloadSize;
    uint32_t cdw12;
    uint32_t cdw13;
};

class DirectiveCmd
{
public:
    DirectiveCmd(UblockSharedPtr dev, spdk_nvme_ctrlr* ctrlr, IIODispatcher* ioDispatcher);
    virtual ~DirectiveCmd(void);

    void EnableDirective(void);
    void ReturnIdentifyParameters(void);

    void AllocateResources(void);
    void ReleaseResources(void);

private:
    void _SubmitUbio(DirReqId reqId);

    static const uint32_t ENABLE_DIRECTIVE = 0x00000101;
    static const uint32_t STREAM_CNT = 3;    // user data, meta data, journal data

    UblockSharedPtr targetDevice;
    spdk_nvme_ctrlr* ctrlr;
    IIODispatcher* ioDispatcher = nullptr;
};
} // namespace pos
#endif // DIRECTIVE_CMD_H_
