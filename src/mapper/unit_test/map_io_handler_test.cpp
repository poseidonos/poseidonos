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

#include "../map_flush_handler.h"
#undef Max
#include "gtest/gtest.h"
#include "src/scheduler/event.h"

namespace ibofos
{
class MapIoHandlerTest : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void _GetDirtyPages(BitMap* validPages);
    void _CheckFlushingPagesBitmap(void);

    Map* map;
    MapHeader* mapHeader;
    MapIoHandler* mapIoHandler;

    static const int MPAGE_SIZE = 4032;
    static const int NUM_PAGES_IN_MAP = MPAGE_SIZE / 8;
};

void
MapIoHandlerTest::SetUp(void)
{
    map = new Map(NUM_PAGES_IN_MAP, MPAGE_SIZE);
    mapHeader = new MapHeader();
    mapIoHandler = new MapIoHandler(map, mapHeader);

    mapHeader->bitmap = new BitMap(NUM_PAGES_IN_MAP);
    mapHeader->bitmap->ResetBitmap();

    mapHeader->dirtyPages = new BitMap(NUM_PAGES_IN_MAP);
    mapHeader->dirtyPages->ResetBitmap();
}

void
MapIoHandlerTest::TearDown(void)
{
    delete map;
    delete mapHeader;

    delete mapIoHandler->flushingPages;
    delete mapIoHandler;
}

void
MapIoHandlerTest::_GetDirtyPages(BitMap* validPages)
{
    mapIoHandler->flushingPages = new BitMap(NUM_PAGES_IN_MAP);
    mapIoHandler->_GetDirtyPages(validPages);
}

void
MapIoHandlerTest::_CheckFlushingPagesBitmap(void)
{
    BitMap* flushingPages = mapIoHandler->flushingPages;

    EXPECT_TRUE(flushingPages->GetNumBitsSet() ==
        mapHeader->dirtyPages->GetNumBitsSet());
    EXPECT_TRUE(flushingPages->GetNumEntry() ==
        mapHeader->dirtyPages->GetNumEntry());
    EXPECT_TRUE(flushingPages->GetNumEntry() != 0);
    EXPECT_TRUE(memcmp(flushingPages->GetMapAddr(),
                    mapHeader->dirtyPages->GetMapAddr(),
                    mapHeader->bitmap->GetNumEntry() * BITMAP_ENTRY_SIZE) == 0);
}

TEST_F(MapIoHandlerTest, GetDirtyPagesWithEmptyMap)
{
    _GetDirtyPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

TEST_F(MapIoHandlerTest, GetDirtyPagesWithFlushedMap)
{
    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum++)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    _GetDirtyPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

TEST_F(MapIoHandlerTest, GetDirtyPages)
{
    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum += 2)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum += 4)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    _GetDirtyPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

} // namespace ibofos
