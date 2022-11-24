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

#include "grpc_pos_io.h"

#include <grpc/status.h>

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/pos_replicator/posreplicator_config.h"
#include "src/pos_replicator/posreplicator_manager.h"

namespace pos
{
::grpc::Status
GrpcPosIo::WriteHostBlocks(::grpc::ServerContext* context,
    const pos_rpc::WriteHostBlocksRequest* request, pos_rpc::WriteHostBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get WriteHostBlocks from grpc client");

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);

    if (ret.ok() == false)
    {
        return ret;
    }

    PosReplicatorManagerSingleton::Instance()->UserVolumeWriteSubmission(request->lsn(), arraySet.second, volumeSet.second);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcPosIo::WriteHostBlocksSync(::grpc::ServerContext* context,
    const pos_rpc::WriteHostBlocksSyncRequest* request, pos_rpc::WriteHostBlocksSyncResponse* response)
{
    // For Test drvier command to test fake-pos
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get WriteHostBlocksSync from grpc client");
    ::grpc::Status ret(::grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
    return ret;
}

::grpc::Status
GrpcPosIo::WriteBlocks(::grpc::ServerContext* context,
    const pos_rpc::WriteBlocksRequest* request, pos_rpc::WriteBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    int dataSize = request->data_size();
    std::shared_ptr<char*> dataList(new char*[dataSize]);
    for (int index = 0; index < dataSize; index++)
    {
        dataList.get()[index] = const_cast<char*>(request->data(index).content().c_str());
    }

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get WriteBlocks from grpc client");

    if (ret.ok() == false)
    {
        return ret;
    }

    PosReplicatorManagerSingleton::Instance()->HAIOSubmission(IO_TYPE::WRITE, arraySet.second, volumeSet.second,
        request->rba(), request->num_blocks(), dataList);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcPosIo::WriteBlocksSync(::grpc::ServerContext* context,
    const pos_rpc::WriteBlocksSyncRequest* request, pos_rpc::WriteBlocksSyncResponse* response)
{
    // For Test drvier command to test fake-pos
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get WriteBlocksSync from grpc client");
    ::grpc::Status ret(::grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
    return ret;
}

::grpc::Status
GrpcPosIo::ReadBlocks(::grpc::ServerContext* context,
    const pos_rpc::ReadBlocksRequest* request, pos_rpc::ReadBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get ReadBlocks from grpc client");

    if (ret.ok() == false)
    {
        return ret;
    }

    PosReplicatorManagerSingleton::Instance()->HAIOSubmission(IO_TYPE::READ, arraySet.second, volumeSet.second,
        request->rba(), request->num_blocks(), nullptr);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcPosIo::ReadBlocksSync(::grpc::ServerContext* context,
    const pos_rpc::ReadBlocksSyncRequest* request, pos_rpc::ReadBlocksSyncResponse* response)
{
    // For Test drvier command to test fake-pos
    pos_rpc::ReadBlocksRequest asyncRequest;
    pos_rpc::ReadBlocksResponse asyncResponse;
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get ReadBlocksSync from grpc client");

    asyncRequest.set_array_name(request->array_name());
    asyncRequest.set_volume_name(request->volume_name());
    asyncRequest.set_rba(request->rba());
    asyncRequest.set_num_blocks(request->num_blocks());

    auto ret = ReadBlocks(context, &asyncRequest, &asyncResponse);
    response->set_result(asyncResponse.result());
    response->set_reason(asyncResponse.reason());

    return ret;
}

::grpc::Status
GrpcPosIo::CompleteHostWrite(::grpc::ServerContext* context, const pos_rpc::CompleteHostWriteRequest* request,
    pos_rpc::CompleteHostWriteResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get CompleteHostWrite from grpc client");

    if (ret.ok() == false)
    {
        return ret;
    }
    PosReplicatorManagerSingleton::Instance()->CompleteUserIO(request->lsn(), arraySet.second, volumeSet.second);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcPosIo::_CheckArgumentValidityAndUpdateIndex(std::pair<std::string, int>& arraySet,
    std::pair<std::string, int>& volumeSet)
{
    int ret = PosReplicatorManagerSingleton::Instance()->ConvertNametoIdx(arraySet, volumeSet);

    if (ret != EID(SUCCESS))
    {
        const string errorMsg = "receive invalid argument";
        POS_TRACE_WARN(ret, "{}", errorMsg);
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, errorMsg);
    }
    return ::grpc::Status::OK;
}
} // namespace pos
