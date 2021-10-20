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

#include "src/qos/processing_manager_array.h"

#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_avg_compute.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosProcessingManagerArray::QosProcessingManagerArray(uint32_t arrayIndex,
    QosContext* qosCtx,
    QosManager* qosManager)
    : qosContext(qosCtx),
    qosManager(qosManager)
{
    arrayId = arrayIndex;
    try
    {
        movingAvgCompute = new MovingAvgCompute(M_QOS_AVERAGE_WINDOW_SIZE);
    }
    catch (bad_alloc& ex)
    {
        assert(0);
    }
    movingAvgCompute->Initilize();
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosProcessingManagerArray::Initilize(void)
{
    movingAvgCompute->Initilize();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosProcessingManagerArray::~QosProcessingManagerArray(void)
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
QosProcessingManagerArray::Execute(uint32_t volId)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolumePolicy = userPolicy.GetAllVolumeUserPolicy();

    uint64_t bw = 0;
    uint64_t iops = 0;
    if (false == allVolParam.VolumeExists(arrayId, volId))
    {
        assert(0);
    }
    VolumeParameter& volParam = allVolParam.GetVolumeParameter(arrayId, volId);
    bw = volParam.GetBandwidth();
    iops = volParam.GetIops();
    iops *= PARAMETER_COLLECTION_INTERVAL;
    if (false != movingAvgCompute->EnqueueAvgData(volId, bw, iops))
    {
        volParam.SetAvgBandwidth(movingAvgCompute->GetMovingAvgBw(volId));
        volParam.SetAvgIops(movingAvgCompute->GetMovingAvgIops(volId) / PARAMETER_COLLECTION_INTERVAL);
    }
    uint64_t avgBw = volParam.GetAvgBandwidth();

    VolumeUserPolicy* volPolicy = allVolumePolicy.GetVolumeUserPolicy(arrayId, volId);
    if (volPolicy->IsMinimumVolume() == true)
    {
        allVolParam.IncrementMinVolumesBw(arrayId, avgBw);
    }
    allVolParam.IncrementTotalBw(arrayId, avgBw);
}
} // namespace pos
