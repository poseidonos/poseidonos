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

#include "mfs_state_mgr.h"

namespace pos
{
const MetaFsSystemStateTransitionEntry MetaFsStateManager::NORMAL_STATE_MACHINE_TABLE[(uint32_t)MetaFsSystemState::Max] =
    {
        // the order of transtion definition entry must be identical of the order of MetaFsSystemState enum
        {MetaFsSystemState::PowerOn, MetaFsSystemState::Init},
        {MetaFsSystemState::Init, MetaFsSystemState::Ready},
        {MetaFsSystemState::Ready, MetaFsSystemState::Ready},
        {MetaFsSystemState::Create, MetaFsSystemState::Create},
        {MetaFsSystemState::Open, MetaFsSystemState::Active},
        {MetaFsSystemState::Quiesce, MetaFsSystemState::Shutdown},
        {MetaFsSystemState::Shutdown, MetaFsSystemState::Shutdown},
        {MetaFsSystemState::Error, MetaFsSystemState::Error},
        {MetaFsSystemState::Recovery, MetaFsSystemState::Active},
        {MetaFsSystemState::Fatal, MetaFsSystemState::Fatal},
        {MetaFsSystemState::Format, MetaFsSystemState::Rebuild},
        {MetaFsSystemState::Rebuild, MetaFsSystemState::Active},
        {MetaFsSystemState::Active, MetaFsSystemState::Active},
};
} // namespace pos
