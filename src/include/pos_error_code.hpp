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

#ifndef __IBOF_ERROR_CODE_H__
#define __IBOF_ERROR_CODE_H__

#include "pos_event_id.hpp"

namespace pos
{
enum IBOF_ERROR_CODE
{
    // --------------IO Path Frontend ----------------
    URAM_SUBMISSION_FAILED_ERROR = -EID(URAM_SUBMISSION_FAILED),
    URAM_SUBMISSION_TIMEOUT_ERROR = -EID(URAM_SUBMISSION_TIMEOUT),
    URAM_COMPLETION_TIMEOUT_ERROR = -EID(URAM_COMPLETION_TIMEOUT),

    // --------------IO Path Backend ----------------
    IOSMHDLR_BUFFER_NOT_ENOUGH_ERROR = -EID(IOSMHDLR_BUFFER_NOT_ENOUGH),
    IOSMHDLR_BUFFER_NOT_ALIGNED_ERROR = -EID(IOSMHDLR_BUFFER_NOT_ALIGNED),
    IOSMHDLR_OPERATION_NOT_SUPPORTED_ERROR = -EID(IOSMHDLR_OPERATION_NOT_SUPPORTED),

    DEVICE_SUBMISSION_FAILED_ERROR = -EID(DEVICE_SUBMISSION_FAILED),
    DEVICE_SUBMISSION_TIMEOUT_ERROR = -EID(DEVICE_SUBMISSION_TIMEOUT),
    DEVICE_COMPLETION_FAILED_ERROR = -EID(DEVICE_COMPLETION_FAILED),
    DEVICE_OPERATION_NOT_SUPPORTED_ERROR = -EID(DEVICE_OPERATION_NOT_SUPPORTED),

    UNVME_SUBMISSION_FAILED_ERROR = -EID(UNVME_SUBMISSION_FAILED),
    UNVME_SUBMISSION_RETRY_EXCEED_ERROR = -EID(UNVME_SUBMISSION_RETRY_EXCEED),

    UNVME_COMPLETION_TIMEOUT_ERROR = -EID(UNVME_COMPLETION_TIMEOUT),
    UNVME_COMPLETION_FAILED_ERROR = -EID(UNVME_COMPLETION_FAILED),
    UNVME_OPERATION_NOT_SUPPORTED_ERROR = -EID(UNVME_OPERATION_NOT_SUPPORTED),
};

} // namespace pos
#endif // __IBOF_ERROR_CODE_H__
