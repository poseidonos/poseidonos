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

#include "src/qos/processing_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/qos_avg_compute.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosProcessingManager::QosProcessingManager(QosContext* qosCtx)
{
    qosContext = qosCtx;
    try
    {
        movingAvgCompute = new MovingAvgCompute(M_QOS_AVERAGE_WINDOW_SIZE);
    }
    catch (bad_alloc& ex)
    {
        assert(0);
    }
    movingAvgCompute->Initilize();
    nextManagerType = QosInternalManager_Unknown;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosProcessingManager::~QosProcessingManager(void)
{
    delete movingAvgCompute;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosProcessingManager::Execute(void)
{
    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        nextManagerType = QosInternalManager_Policy;
        return;
    }
    std::map<uint32_t, uint32_t> activeVolMap = qosContext->GetActiveVolumes();
    if (0 == activeVolMap.size())
    {
        movingAvgCompute->Initilize();
        nextManagerType = QosInternalManager_Policy;
        return;
    }

    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    for (map<uint32_t, uint32_t>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        uint32_t volId = it->first;
        uint64_t bw = 0;
        uint64_t iops = 0;
        if (false == allVolParam.VolumeExists(volId))
        {
            assert(0);
        }
        VolumeParameter& volParam = allVolParam.GetVolumeParameter(volId);
        bw = volParam.GetBandwidth();
        iops = volParam.GetIops();
        iops *= PARAMETER_COLLECTION_INTERVAL;
        if (false != movingAvgCompute->EnqueueAvgData(volId, bw, iops))
        {
            volParam.SetAvgBandwidth(movingAvgCompute->GetMovingAvgBw(volId));
            volParam.SetAvgIops(movingAvgCompute->GetMovingAvgIops(volId) / PARAMETER_COLLECTION_INTERVAL);
        }
    }
    _SetNextManagerType();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosProcessingManager::_SetNextManagerType(void)
{
    nextManagerType = QosInternalManager_Policy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosInternalManagerType
QosProcessingManager::GetNextManagerType(void)
{
    return nextManagerType;
}
} // namespace pos
