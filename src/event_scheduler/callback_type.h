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


namespace pos
{
class SystemTimeoutChecker;
class EventScheduler;
enum CallbackType
{
    CallbackType_Unknown = 0,
    CallbackType_AdminCommandCompleteHandler,
    CallbackType_DiskSmartCompleteHandler,
    CallbackType_SmartLogUpdateRequest,
    CallbackType_UramRestoreCompletion,
    CallbackType_CopierReadCompletion,
    CallbackType_GcFlushCompletion,
    CallbackType_ReverseMapLoadCompletion,
    CallbackType_StripeCopySubmission,
    CallbackType_FlushReadCompletion,
    CallbackType_StripeMapUpdateRequest = 10,
    CallbackType_RebuildReadCompleteHandler,
    CallbackType_RebuildReadIntermediateCompleteHandler,
    CallbackType_AioCompletion,
    CallbackType_AdminCompletion,
    CallbackType_BlockMapUpdateRequest,
    CallbackType_ReadCompletionForPartialWrite,
    CallbackType_ReadCompletion,
    CallbackType_WriteCompletion,
    CallbackType_ArrayUnlocking,
    CallbackType_AbortCompletionHandler = 20,
    CallbackType_InternalReadCompletion,
    CallbackType_InternalWriteCompletion,
    CallbackType_SyncIoCompletion,
    CallbackType_MssIoCompletion,
    CallbackType_NvramIoCompletion,
    CallbackType_UpdateDataCompleteHandler,
    CallbackType_UpdateDataHandler,
    CallbackType_BlockMapUpdateRequestCompletion,
    CallbackType_StripeMapUpdateCompletion,
    CallbackType_FlushCompletion = 30,
    CallbackType_WriteThroughStripeLoad,
    CallbackType_PosReplicatorIOCompletion,
    Total_CallbackType_Cnt
};

}