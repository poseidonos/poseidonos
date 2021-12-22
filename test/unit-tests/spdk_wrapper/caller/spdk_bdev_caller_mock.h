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

#include <gmock/gmock.h>

#include "src/spdk_wrapper/caller/spdk_bdev_caller.h"

namespace pos
{
class MockSpdkBdevCaller : public SpdkBdevCaller
{
public:
    MOCK_METHOD(int, SpdkBdevQueueIoWait, (struct spdk_bdev *bdev, struct spdk_io_channel *ch, struct spdk_bdev_io_wait_entry *entry), (override));
    MOCK_METHOD(struct spdk_bdev*, SpdkBdevFirst, ());
    MOCK_METHOD(struct spdk_bdev*, SpdkBdevNext, (struct spdk_bdev* bdev));
    MOCK_METHOD(struct spdk_bdev*, SpdkBdevGetByName, (const char* name));
    MOCK_METHOD(int, SpdkBdevOpenExt, (const char* bdev_name, bool write, spdk_bdev_event_cb_t event_cb, void* event_ctx, struct spdk_bdev_desc** desc));
    MOCK_METHOD(struct spdk_io_channel*, SpdkBdevGetIoChannel, (struct spdk_bdev_desc* desc));
    MOCK_METHOD(void, SpdkBdevClose, (struct spdk_bdev_desc* desc));
    MOCK_METHOD(int, SpdkBdevRead, (struct spdk_bdev_desc *desc, struct spdk_io_channel *ch, void *buf, uint64_t offset, uint64_t nbytes, spdk_bdev_io_completion_cb cb, void *cb_arg));
    MOCK_METHOD(int, SpdkBdevWrite, (struct spdk_bdev_desc *desc, struct spdk_io_channel *ch, void *buf, uint64_t offset, uint64_t nbytes, spdk_bdev_io_completion_cb cb, void *cb_arg));
};

} // namespace pos
