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

#include <gtest/gtest.h>
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
// This is to test out and understand proto's API
TEST(SegmentInfoDataProto, testIfCompleteProtoIsSerialized)
{
    // Given a proto with full information
    pos_bc::SegmentInfoDataProto proto;
    proto.set_valid_block_count(1);
    proto.set_occupied_stripe_count(2);
    proto.set_state( (pos_bc::SegmentState) SegmentState::FREE );

    char ioBuffer[SegmentInfoData::ONSSD_SIZE];

    // When 1: we pass in a fixed-size buffer larger than message size
    int ret = proto.SerializeToArray(ioBuffer, SegmentInfoData::ONSSD_SIZE);

    // Then 1: serialization succeeds, but returns non-zero
    EXPECT_NE(0, ret);
    
    // TODO(yyu) - find a way to make SerializeToArray return 0! "When 2/Then 2" doesn't work as I expect.
    // When 2: we pass in exactly sized buffer
    /*char* exactlySizedBuffer = new char[proto.ByteSizeLong()];
    ret = proto.SerializeToArray(ioBuffer, proto.ByteSizeLong());
    delete [] exactlySizedBuffer;

    // Then 2
    EXPECT_EQ(0, ret);*/
}

TEST(SegmentInfoDataProto, testIfIncompleteProtoFailedToBeSerialized)
{
    // Given a proto missing one field "state"
    pos_bc::SegmentInfoDataProto proto;
    proto.set_valid_block_count(3);
    proto.set_occupied_stripe_count(4);
    char ioBuffer[SegmentInfoData::ONSSD_SIZE];

    // When
    int ret = proto.SerializeToArray(ioBuffer, SegmentInfoData::ONSSD_SIZE);

    // Then
    EXPECT_NE(0, ret);
}

TEST(SegmentInfoDataProto, testIfProtoSizeChangesAsDataIsSet)
{
    // Given a proto 
    pos_bc::SegmentInfoDataProto proto;

    // When 1: a new/clean proto is given

    // Then 1: its size should be zero
    EXPECT_EQ(0, proto.ByteSizeLong());

    // When 2: all fields are set to 0
    proto.set_valid_block_count(0);
    proto.set_occupied_stripe_count(0);
    proto.set_state( (pos_bc::SegmentState) 0 );

    // Then 2: its size should "still" be zero
    EXPECT_EQ(0, proto.ByteSizeLong());

    // When 3: the valid block count is set to non-zero
    proto.set_valid_block_count(1);

    // Then 3: its size should be a little larger than zero (looks like proto encodes the field in 2-bytes)
    EXPECT_EQ(2, proto.ByteSizeLong());

    // When 4: the occupied stripe count is set to non-zero
    proto.set_occupied_stripe_count(2);

    // Then 4: its size should be a little larger than before
    EXPECT_EQ(4, proto.ByteSizeLong());

    // When 5: the state is set to non-zero
    proto.set_state( (pos_bc::SegmentState) 1 );

    // Then 5: its size should be a little larger than before
    EXPECT_EQ(6, proto.ByteSizeLong());
}
}