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

#include "grpc_replication_controller.h"

#include <grpc/status.h>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/pos_replicator/posreplicator_manager.h"
#include "src/pos_replicator/posreplicator_status.h"

namespace pos
{
GrpcReplicationController::GrpcReplicationController(PosReplicatorManager* replicatorManager)
: replicatorManager(replicatorManager)
{
}

::grpc::Status
GrpcReplicationController::StartVolumeSync(
    ::grpc::ServerContext* context,
    const pos_rpc::StartVolumeSyncRequest* request,
    pos_rpc::StartVolumeSyncResponse* response)
{
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG), "Get StartVolumeSync from grpc client");

    bool is_primary = request->is_primary();
    if (is_primary)
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopy);
    }
    else
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_SecondaryVolumeCopy);
    }

    response->set_result(pos_rpc::PosResult::SUCCESS);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcReplicationController::FinishVolumeSync(
    ::grpc::ServerContext* context,
    const pos_rpc::FinishVolumeSyncRequest* request,
    pos_rpc::FinishVolumeSyncResponse* response)
{
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG), "Get FinishVolumeSync from grpc client");
    ReplicatorStatus status = replicatorManager->GetVolumeCopyStatus();
    if (status == ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopyWriteSuspend)
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_PrimaryLiveReplication);
    }
    else if (status == ReplicatorStatus::VOLUMECOPY_SecondaryVolumeCopy)
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_SecondaryLiveReplication);
    }
    else
    {
        assert(false);
    }

    response->set_result(pos_rpc::PosResult::SUCCESS);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcReplicationController::SuspendWrite(
    ::grpc::ServerContext* context,
    const pos_rpc::SuspendWriteRequest* request,
    pos_rpc::SuspendWriteResponse* response)
{
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG), "Get SuspendWrite from grpc client");
    ReplicatorStatus status = replicatorManager->GetVolumeCopyStatus();
    if (status == ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopy)
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopyWriteSuspend);
    }
    else if (status == ReplicatorStatus::VOLUMECOPY_SecondaryVolumeCopy)
    {
        assert(false);
    }
    else
    {
        assert(false);
    }

    response->set_result(pos_rpc::PosResult::SUCCESS);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcReplicationController::ResumeWrite(
    ::grpc::ServerContext* context,
    const pos_rpc::ResumeWriteRequest* request,
    pos_rpc::ResumeWriteResponse* response)
{
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG), "Get ResumeWrite from grpc client");
    ReplicatorStatus status = replicatorManager->GetVolumeCopyStatus();
    if (status == ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopyWriteSuspend)
    {
        replicatorManager->SetVolumeCopyStatus(ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopy);
    }
    else if (status == ReplicatorStatus::VOLUMECOPY_SecondaryVolumeCopy)
    {
        assert(false);
    }
    else
    {
        assert(false);
    }

    response->set_result(pos_rpc::PosResult::SUCCESS);
    return ::grpc::Status::OK;
}
} // namespace pos
