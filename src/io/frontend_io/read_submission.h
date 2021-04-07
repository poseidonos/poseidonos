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

#pragma once

#include "src/include/address_type.h"
#include "src/io/frontend_io/read_completion.h"
#include "src/io/general_io/block_alignment.h"
#include "src/io/general_io/io_controller.h"
#include "src/io/general_io/merger.hpp"
#include "src/io/general_io/translator.h"
#include "src/scheduler/event.h"

namespace ibofos
{
class EventArgument;
class VolumeIo;

class ReadSubmission : public IOController, public Event
{
public:
    ReadSubmission(VolumeIoSmartPtr volumeIo);
    ~ReadSubmission(void) override;
    bool Execute(void) override;

private:
    void _PrepareSingleBlock(void);
    void _PrepareMergedIo(void);
    void _MergeBlock(uint32_t blocIndex);
    void _ProcessMergedIo(void);
    void _ProcessVolumeIo(uint32_t volumeIoIndex);

    BlockAlignment blockAlignment;
    Merger<ReadCompletion> merger;
    Translator translator;
    VolumeIoSmartPtr volumeIo;
};

} // namespace ibofos
