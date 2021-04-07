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

#include "../memory_checker.h"

class Dummy
{
private:
    int a;
    int b;
};

void
test_fail_memory_corruption(void)
{
    Dummy* ptr = new Dummy;
    *(uint64_t*)((char*)ptr + sizeof(Dummy)) = 0x123123;
    delete ptr;
}

void
test_fail_single_double_free(void)
{
    Dummy* ptr = new Dummy;
    delete ptr;
    delete ptr;
}

void
test_fail_complex_double_free(void)
{
    Dummy* ptr[1024000];
    int i;
    for (i = 0; i < 1024000; i++)
    {
        ptr[i] = new Dummy;
    }
    for (i = 0; i < 1024000; i++)
    {
        delete (ptr[i]);
    }
    for (i = 0; i < 1024000; i++)
    {
        ptr[i] = new Dummy;
    }
    for (i = 0; i < 1024000; i++)
    {
        delete (ptr[i]);
    }
    delete (ptr[0]);
}

// This test needs to gdb.
// delete for similar free block.
// this test will split one free block as two. (front some thing and back some thing)
// total size of split size  = PADDING + Dummy class size;
void
test_erase_list_single(void)
{
    Dummy* ptr[2];
    int i;
    for (i = 0; i < 2; i++)
    {
        ptr[i] = new Dummy;
    }
    delete (ptr[0]);
    ibofos::MemoryChecker::Enable(false);
    printf("%ld %ld \n", (uint64_t)ptr[0], sizeof(Dummy));
    ibofos::MemoryChecker::EraseFromFreeList((uint64_t)((char*)ptr[0] + 1), 4);
    while (1)
    {
        // Do nothing
    }
}

// This test needs to gdb.
// delete for similar free block.
// this test will split ptr[0] and ptr[2]
// total size of split size  = PADDING + Dummy class size
void
test_erase_list_multiple(void)
{
    Dummy* ptr[3];
    int i;
    for (i = 0; i < 3; i++)
    {
        ptr[i] = new Dummy;
    }
    delete (ptr[0]);
    delete (ptr[1]);
    delete (ptr[2]);
    ibofos::MemoryChecker::Enable(false);
    printf("%ld %ld \n", (uint64_t)ptr[2], sizeof(Dummy));
    // You need to expermentally choose the size (96+64)
    ibofos::MemoryChecker::EraseFromFreeList((uint64_t)((char*)ptr[0] + 1), 96 + 64);
    while (1)
    {
        // Do nothing
    }
}

int
main()
{
    ibofos::MemoryChecker::Enable(true);
    // All should be segmentation failure,
    // So, please check one by one.
    test_fail_memory_corruption();
    test_fail_single_double_free();
    test_fail_complex_double_free();
    test_erase_list_single();
    test_erase_list_multiple();
    ibofos::MemoryChecker::Enable(false);
}
