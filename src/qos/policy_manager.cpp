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

#include "src/qos/policy_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/qos/correction_manager.h"
#include "src/qos/event_cpu_policy.h"
#include "src/qos/policy_handler.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/volume_policy.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosPolicyManager::QosPolicyManager(QosContext* qosCtx, QosManager* qosManager)
    : qosContext(qosCtx),
    qosManager(qosManager)
{
    nextManagerType = QosInternalManager_Unknown;
    eventCpuPolicy = new EventCpuPolicy(qosCtx);
    volumePolicy = new VolumePolicy(qosCtx);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosPolicyManager::~QosPolicyManager(void)
{
    delete eventCpuPolicy;
    delete volumePolicy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::Execute(void)
{
    qosContext->SetApplyCorrection(false);
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    qosCorrection.SetCorrectionType(QosCorrection_EventThrottle, false);
    qosCorrection.SetCorrectionType(QosCorrection_EventWrr, false);
    qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, false);
    if (true == qosManager->IsFeQosEnabled())
    {
        volumePolicy->HandlePolicy();
    }

    eventCpuPolicy->HandlePolicy();
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
QosPolicyManager::_SetNextManagerType(void)
{
    qosContext->IncrementCorrectionCycle();
    if (true == qosContext->GetApplyCorrection())
    {
        nextManagerType = QosInternalManager_Correction;
    }
    else
    {
        nextManagerType = QosInternalManager_Monitor;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosInternalManagerType
QosPolicyManager::GetNextManagerType(void)
{
    return nextManagerType;
}
} // namespace pos
