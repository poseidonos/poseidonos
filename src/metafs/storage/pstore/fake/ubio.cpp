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

#include "src/io/general_io/ubio.h"

#include "src/include/memory.h"
#include "src/include/meta_const.h"

using namespace ibofos;
/**
 * Only need constructor and destructos of Ubio and UbioVector
 */

UbioVector::UbioVector(void* mem, uint32_t page_cnt)
: ownership(false)
{
    if (page_cnt == 0)
        return;

    if (mem == nullptr)
    {
        ownership = true;
        mem = ibofos::Memory<ibofos::SectorSize>::Alloc(page_cnt * SectorsPerPage());
    }

    char* addr = reinterpret_cast<char*>(mem);
    for (uint32_t i = 0; i < page_cnt; i++)
    {
        vec_.push_back(addr + i * ibofos::PageSize);
    }
}

UbioVector::~UbioVector(void)
{
    if (ownership)
        ibofos::Memory<>::Free(vec_[0]);
}

void
UbioVector::AddPage(void* addr)
{
    vec_.push_back(addr);
}

uint64_t
UbioVector::Byte(void)
{
    return ibofos::PageSize * vec_.size();
}

void*
UbioVector::Page(uint32_t page_index)
{
    assert(page_index < vec_.size());
    return vec_[page_index];
}

Ubio::Ubio(void* buffer, uint32_t unitCount)
: dir(UbioDir::Read),
  sync(false),
  volume_id(-1),
  dev(nullptr),
  error(0),
  vec_idx(0),
  vec_off(0),
  size(BYTES_PER_UNIT * unitCount),
  address(0),
  stream_id(0),
  next(nullptr),
  endio(nullptr),
  syncDone(false),
  remaining(1),
  iov(nullptr),
  iov_cnt(0),
  iov_pages(nullptr)
{
    uint32_t blockCount = DIV_ROUND_UP(unitCount, UNITS_PER_BLOCK);
    vec = std::shared_ptr<UbioVector>(new UbioVector(buffer, blockCount));
}

Ubio::Ubio(const Ubio& ubio)
{
    dir = ubio.dir;
    sync = false;
    volume_id = ubio.volume_id;
    dev = ubio.dev;
    error.store(0);

    vec = ubio.vec;
    vec_idx = ubio.vec_idx;
    vec_off = ubio.vec_off;
    size = ubio.size;
    address = ubio.address;
    stream_id = 0;

    next = nullptr;
    endio = ubio.endio;
    syncDone = false;
    remaining.store(1);

    private_ = ubio.private_;

    iov = nullptr;
    iov_cnt = 0;
    iov_pages = nullptr;
}

Ubio::~Ubio(void)
{
    vec.reset();
    if (iov)
        free(iov);
    if (iov_pages)
        free(iov_pages);
}
