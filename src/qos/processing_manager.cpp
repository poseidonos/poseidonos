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

#include "src/qos/processing_manager.h"

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
QosProcessingManager::QosProcessingManager(QosContext* qosCtx)
{
    qosContext = qosCtx;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosProcessingManagerArray[i] = new QosProcessingManagerArray(i, qosCtx);
    }
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
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        delete qosProcessingManagerArray[i];
    }
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
    std::map<uint32_t, uint32_t> activeVolMap = qosContext->GetActiveVolumes();

    if (0 == activeVolMap.size())
    {
        uint32_t numberArrays = QosManagerSingleton::Instance()->GetNumberOfArrays();
        for (uint32_t i = 0; i < numberArrays; i++)
        {
            qosProcessingManagerArray[i]->Initilize();
        }
        nextManagerType = QosInternalManager_Policy;
        return;
    }

    for (map<uint32_t, uint32_t>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        uint32_t volId = it->first;
        uint32_t arrVolId = volId % MAX_VOLUME_COUNT;
        uint32_t arrayId = volId / MAX_VOLUME_COUNT;
        qosProcessingManagerArray[arrayId]->Execute(arrVolId);
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
