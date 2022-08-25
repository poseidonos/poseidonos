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

#pragma once

#include <string>
#include <thread>
#include <unordered_map>

#include "posreplicator_config.h"
#include "proto/generated/cpp/pos_rpc.grpc.pb.h"
#include "proto/generated/cpp/pos_rpc.pb.h"
#include "spdk/pos.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/callback.h"
#include "src/include/array_mgmt_policy.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/lib/singleton.h"
#include "src/pos_replicator/grpc_publisher.h"
#include "src/pos_replicator/grpc_subscriber.h"
#include "src/pos_replicator/replicator_volume_subscriber.h"
#include "src/sys_event/volume_event.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/i_volume_info_manager.h"
#include "src/volume/volume_base.h"

namespace pos
{
class PosReplicatorManager
{
public:
    PosReplicatorManager(void);
    ~PosReplicatorManager(void);

    void Init(GrpcPublisher* publisher, GrpcSubscriber* subscriber);
    void Dispose(void);
    void Clear(void);

    int Register(int arrayId, ReplicatorVolumeSubscriber* volumeSubscriber);
    void Unregister(int arrayId);
    ReplicatorVolumeSubscriber* GetReplicatorVolumeSubscriber(int arrayId);

    int NotifyNewUserIORequest(pos_io io);
    int CompleteUserIO(uint64_t lsn, int arrayId, int volumeId);

    int UserVolumeWriteSubmission(uint64_t lsn, int arrayId, int volumeId);

    int HAIOSubmission(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t num_blocks, void* data);
    void HAIOCompletion(uint64_t lsn, pos_io io, VolumeIoSmartPtr volumeIo);
    void HAWriteCompletion(uint64_t lsn, pos_io io);
    void HAReadCompletion(uint64_t lsn, pos_io io, VolumeIoSmartPtr volumeIo);

    int ConvertArrayIdtoArrayName(int arrayId, std::string& arrayName);
    int ConvertVolumeIdtoVolumeName(int volumeId, int arrayId, std::string& volumeName);
    int ConvertArrayNametoArrayId(std::string arrayName);
    int ConvertVolumeNametoVolumeId(std::string volumeName, int arrayId);

    void AddDonePOSIoRequest(uint64_t lsn, VolumeIoSmartPtr volumeIo);

    int ConvertNametoIdx(std::pair<std::string, int> arraySet, std::pair<std::string, int> volumeSet);

protected:
    std::thread* posRepilicatorManagerServerThread;
    std::unique_ptr<::grpc::Server> server;

private:
    void _AddWaitPOSIoRequest(uint64_t lsn, pos_io io);

    void _MakeIoRequest(GrpcCallbackType callbackType, pos_io io, uint64_t lsn);
    pos_io _MakePosIo(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t num_blocks);

    std::unordered_map<uint64_t, pos_io> waitPosIoRequest[ArrayMgmtPolicy::MAX_ARRAY_CNT][MAX_VOLUME_COUNT];
    std::unordered_map<uint64_t, VolumeIoSmartPtr> donePosIoRequest[ArrayMgmtPolicy::MAX_ARRAY_CNT][MAX_VOLUME_COUNT];

    int volumeSubscriberCnt;
    ReplicatorVolumeSubscriber* items[ArrayMgmtPolicy::MAX_ARRAY_CNT];
    std::mutex listMutex;

    vector<std::pair<int, string>> arrayConvertTable;

    GrpcPublisher* grpcPublisher;
    GrpcSubscriber* grpcSubscriber;
};

class PosReplicatorIOCompletion : public Callback, public std::enable_shared_from_this<PosReplicatorIOCompletion>
{
public:
    PosReplicatorIOCompletion(GrpcCallbackType grpcCallbackType, VolumeIoSmartPtr inputVolumeIo, uint64_t lsn_, pos_io& posIo, CallbackSmartPtr originCallback_);
    ~PosReplicatorIOCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;

    GrpcCallbackType grpcCallbackType;
    VolumeIoSmartPtr volumeIo;
    uint64_t lsn;
    pos_io& posIo;
    CallbackSmartPtr originCallback;
};

// [To do] grpc contact point for 2node-HA
using PosReplicatorManagerSingleton = Singleton<PosReplicatorManager>;
} // namespace pos
