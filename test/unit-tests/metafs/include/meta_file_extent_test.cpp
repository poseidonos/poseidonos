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

#include "src/metafs/include/meta_file_extent.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileExtent, Compare1)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_TRUE((a == b));
    EXPECT_FALSE((a == c));
    EXPECT_FALSE((a == d));
}

TEST(MetaFileExtent, Compare2)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_FALSE((a < b));
    EXPECT_TRUE((a < c));
}

TEST(MetaFileExtent, Compare3)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_FALSE((a > d));
    EXPECT_TRUE((c > a));
}

TEST(MetaFileExtent, Setter)
{
    MetaFileExtent a;
    MetaFileExtent b;

    a.SetStartLpn(1);
    b.SetStartLpn(1);

    a.SetCount(1);
    b.SetCount(1);

    EXPECT_TRUE((a == b));
}

TEST(MetaFileExtent, Getter)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);

    EXPECT_TRUE(a.GetStartLpn() == b.GetStartLpn());
    EXPECT_TRUE(a.GetCount() == b.GetCount());
    EXPECT_TRUE(a.GetLast() == b.GetLast());
}

} // namespace pos
